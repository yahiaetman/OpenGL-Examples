#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <texture/texture-utils.h>
#include <camera/camera.hpp>
#include <camera/controllers/fly_camera_controller.hpp>

// In the previous examples, we used to store just the position and color for each vertex.
// Now we need to identify which part of the texture will be displayed on the triangle.
// To do this, we need to store an extra field called "texture coordinates" which will identify the location of the vertex in the texture space.
// The texture coordinates span from [0,0] (bottom left corner) to [1,1] (top right corner). This allows our meshes to work with textures of different sizes without changing the vertex data.
struct Vertex {
    glm::vec3 position;
    glm::vec<4, glm::uint8, glm::defaultp> color;
    glm::vec2 tex_coord;
};

// This example demonstrates how texture sampling is done for more complex cases where texelFetch is not enough to get visually satisfactory results.
class TextureSamplingApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh model;

    std::vector<Vertex> vertices = {
            {{-0.5, -0.5, 0.0},{255, 255, 255, 255}, {-1, -1}},
            {{0.5, -0.5, 0.0},{255, 255, 255, 255}, {2, -1}},
            {{0.5, 0.5, 0.0},{255, 255, 255, 255}, {2, 2}},
            {{-0.5, 0.5, 0.0},{255, 255, 255, 255}, {-1, 2}}
    };
    std::vector<uint16_t> elements = { 0, 1, 2, 2, 3, 0 };

    std::unordered_map<std::string, GLuint> textures;
    std::string current_texture_name;

    // These are the texture filtering settings that we will use for our sampling.
    GLenum magnification_filter = GL_NEAREST, minification_filter = GL_NEAREST;
    GLenum wrap_s = GL_CLAMP_TO_EDGE, wrap_t = GL_CLAMP_TO_EDGE;
    glm::vec4 border_color = {1,1,1,1};
    GLfloat max_anisotropy = 1.0f;

    our::Camera camera;
    our::FlyCameraController camera_controller;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Texture Sampling", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        // The vertex shader now has 1 extra task; to pass the texture coordinates from the attributes to the fragment shader.
        program.attach("assets/shaders/ex22_texture_sampling/transform.vert", GL_VERTEX_SHADER);
        // This fragment shader will the function "texture" that applies filtering and returns a color from the texture based on the texture coordinates.
        program.attach("assets/shaders/ex22_texture_sampling/texture.frag", GL_FRAGMENT_SHADER);
        program.link();

        model.create({
           [](){
               glEnableVertexAttribArray(0);
               glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
               glEnableVertexAttribArray(1);
               glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), (void*)offsetof(Vertex, color));
               glEnableVertexAttribArray(2);
               glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));
           }
        });

        GLuint texture;

        // Similar to the previous example, we create a few textures to display on our mesh.
        glGenTextures(1, &texture);
        uint8_t pixel_data[] = {
                255,   0,   0, 255,
                0, 255,   0, 255,
                0,   0, 255, 255,
                255, 255,   0, 255,
        };
        glBindTexture(GL_TEXTURE_2D, texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        textures["colors"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::checkerBoard(texture, {6,6}, {3,3}, {255, 255, 255, 255}, {64, 64, 64, 255});
        textures["checkerboard"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/color-grid.png");
        textures["color-grid"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/moon.jpg");
        textures["moon"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/monarch.png");
        textures["monarch"] = texture;

        current_texture_name = "color-grid";

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({0, 0, 1});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        camera_controller.initialize(this, &camera);
        camera_controller.setFieldOfViewSensitivity(0.05f );
        camera_controller.setPositionSensitivity(glm::vec3(0.5f));

        glClearColor(0, 0, 0, 1);
    }

    void onDraw(double deltaTime) override {
        camera_controller.update(deltaTime);

        model.setVertexData(0, vertices, GL_STREAM_DRAW);
        model.setElementData( elements, GL_STREAM_DRAW);

        glUseProgram(program);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program.set("tint", glm::vec4(1, 1, 1, 1));

        program.set("transform", camera.getVPMatrix());

        GLuint texture = textures[current_texture_name];

        // Here we bind the texture and send its unit to the "sampler" uniform variable.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        program.set("sampler", 0);

        // Here we define a group of texture parameters that will affect the sampling process.
        // This parameter defines the magnification filter. This filter will be used when the texture is magnified (1 texture pixel covers more than 1 screen pixel).
        // The possible values are:
        //  -   GL_NEAREST which returns the color from the nearest texture pixel.
        //  -   GL_LINEAR which linearly interpolates the colors of the 4 surrounding pixels.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnification_filter);
        // This parameter defines the minification filter. This filter will be used when the texture is minified (1 screen pixel covers more than 1 texture pixel).
        // The possible values are:
        //  -   Similar to magnification, we have 2 modes that ignores the mip maps:
        //      -   GL_NEAREST which returns the color from the nearest texture pixel.
        //      -   GL_LINEAR which linearly interpolates the colors of the 4 surrounding pixels.
        //  -   Specific to minification, we have 4 modes that follow the form GL_*_MIPMAP_*
        //      where the 1st wild card specifies the filtering within a texture and the 2nd wild card specifies the filtering between mip levels.
        //      -   GL_NEAREST_MIPMAP_NEAREST which returns the color of the nearest pixel in the nearest mip level.
        //      -   GL_NEAREST_MIPMAP_LINEAR which linearly interpolates between the colors of the nearest pixel in each of the surrounding mip levels.
        //      -   GL_LINEAR_MIPMAP_NEAREST which applies linear-filtering in the nearest mip level.
        //      -   GL_LINEAR_MIPMAP_LINEAR which linearly interpolates the results of the linear filtering in each of the surrounding mip levels.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minification_filter);
        // These pair of parameters define the Wrap mode along the 2 texture coordinates: S and T.
        // The Wrap mode specifies how to deal with texture coordinates that lie outside the range [0, 1].
        // The possible values are:
        //  -   GL_REPEAT: this mode keeps adding or subtracting 1 from the coordinate till it return to the range [0, 1]. This leads to a texture repeating effect since for example, the coordinates -0.2, 0.8, 1.8, 2.8, and so on all maps to same location in the texture.
        //  -   GL_MIRRORED_REPEAT: similar to GL_REPEAT but each repetition is the mirrored version of the ones surrounding it.
        //  -   GL_CLAMP_TO_BORDER: this mode returns the border color for coordinates that exceed the range.
        //  -   GL_CLAMP_TO_EDGE: this modes clamps the textures coordinates to the range [0, 1] before sampling. So any value below 0 will be 0 and any value above 1 will be 1.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
        // This parameter identifies the border color to be used with the wrap mode GL_CLAMP_TO_BORDER
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));
        // This parameter identifies the maximum anisotropy which can be used to enable and specify the quality of the anisotropic filtering.
        // Minimum value is 1.0 which is equivalent to disabling the anisotropic filtering.
        // Maximum value depends on the GPU and can be enumerated using the glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy_upper_bound);
        // Anisotropic filtering allows the use of mip maps on samples where the derivatives vary vastly on the X & Y directions of the screen space without over-blurring the result.
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);

        model.draw();

    }

    void onDestroy() override {
        program.destroy();
        model.destroy();
    }

    void onImmediateGui(ImGuiIO &io) override {

        ImGui::Begin("Controls");
        GLenum primitive_mode = model.getPrimitiveMode();
        our::OptionMapCombo("Primitive Type", primitive_mode, our::gl_enum_options::primitives);
        model.setPrimitiveMode(primitive_mode);

        bool use_elements = model.isUsingElements();
        ImGui::Checkbox("Use Elements", &use_elements);
        model.setUseElements(use_elements);

        ImGui::Separator();

        if(ImGui::BeginCombo("Texture", current_texture_name.c_str())){
            for(const auto& [name, texture]: textures){
                bool selected = current_texture_name == name;
                if(ImGui::Selectable(name.c_str(), selected))
                    current_texture_name = name;
                if(selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        our::OptionMapCombo("Magnification Filter", magnification_filter, our::gl_enum_options::texture_magnification_filters);
        our::OptionMapCombo("Minification Filter", minification_filter, our::gl_enum_options::texture_minification_filters);
        our::OptionMapCombo("Wrap S", wrap_s, our::gl_enum_options::texture_wrapping_modes);
        our::OptionMapCombo("Wrap T", wrap_t, our::gl_enum_options::texture_wrapping_modes);
        ImGui::ColorEdit4("Border Color", glm::value_ptr(border_color));

        ImGui::Separator();

        GLfloat max_anisotropy_upper_bound;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy_upper_bound);
        ImGui::DragFloat("Maximum Anisotropy", &max_anisotropy, 0.1f, 1.0f, max_anisotropy_upper_bound);
        ImGui::Text("Maximum Anisotropy Upper Bound: %f", max_anisotropy_upper_bound);

        ImGui::End();

        ImGui::Begin("Vertices");
        our::ReorderableList(std::begin(vertices), std::end(vertices),
                             [](size_t index, Vertex& vertex){
                                 ImGui::Text("Vertex %zu", index);
                                 ImGui::DragFloat3("Position", glm::value_ptr(vertex.position), 0.01);
                                 our::ColorEdit4U8("Color", glm::value_ptr(vertex.color));
                                 ImGui::DragFloat2("Texture Coordinates", glm::value_ptr(vertex.tex_coord));
                             },
                             [&](size_t index){ vertices.insert(std::begin(vertices) + index, Vertex()); },
                             [&](size_t index){ vertices.erase(std::begin(vertices) + index); });
        ImGui::End();

        ImGui::Begin("Elements");
        int max_element = (int)vertices.size() - 1;
        float speed = 1.0f / (float)(max_element + 1);
        our::ReorderableList(std::begin(elements), std::end(elements),
                             [&](size_t index, uint16_t & element){
                                 std::string str_id = std::to_string(index);
                                 int element_i32 = element;
                                 ImGui::DragInt(str_id.c_str(), &element_i32, speed, 0, max_element);
                                 element = element_i32;
                             },
                             [&](size_t index){ elements.insert(std::begin(elements) + index, 0); },
                             [&](size_t index){ elements.erase(std::begin(elements) + index); });
        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return TextureSamplingApplication().run();
}
