#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <texture/texture-utils.h>
#include <camera/camera.hpp>
#include <camera/controllers/fly_camera_controller.hpp>

#include <glm/gtx/euler_angles.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec<4, glm::uint8, glm::defaultp> color;
    glm::vec2 tex_coord;
};

class TextureApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh model;

    std::vector<Vertex> vertices = {
            {{-0.5, -0.5, 0.0},{255, 255, 255, 255}, {0, 0}},
            {{0.5, -0.5, 0.0},{255, 255, 255, 255}, {1, 0}},
            {{0.5, 0.5, 0.0},{255, 255, 255, 255}, {1, 1}},
            {{-0.5, 0.5, 0.0},{255, 255, 255, 255}, {0, 1}}
    };
    std::vector<uint16_t> elements = { 0, 1, 2, 2, 3, 0 };

    GLuint texture;
    GLenum magnification_filter = GL_NEAREST, minification_filter = GL_NEAREST;
    GLenum wrap_s = GL_CLAMP_TO_EDGE, wrap_t = GL_CLAMP_TO_EDGE;
    glm::vec4 border_color = {1,1,1,1};

    our::Camera camera;
    our::FlyCameraController camera_controller;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Textures", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex21_texture/transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex21_texture/texture.frag", GL_FRAGMENT_SHADER);
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

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/ex21_texture/color-grid.png");

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({0, 0, 1});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        camera_controller.initialize(this, &camera);

        glClearColor(0, 0, 0, 0);

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

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        program.set("sampler", 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnification_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minification_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));

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

        our::OptionMapCombo("Magnification Filter", magnification_filter, our::gl_enum_options::texture_magnification_filters);
        our::OptionMapCombo("Minification Filter", minification_filter, our::gl_enum_options::texture_minification_filters);
        our::OptionMapCombo("Wrap S", wrap_s, our::gl_enum_options::texture_wrapping_modes);
        our::OptionMapCombo("Wrap T", wrap_t, our::gl_enum_options::texture_wrapping_modes);
        ImGui::ColorEdit4("Border Color", glm::value_ptr(border_color));
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
    return TextureApplication().run();
}
