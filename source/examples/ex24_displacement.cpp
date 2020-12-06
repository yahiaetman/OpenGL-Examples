#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <mesh/mesh-utils.hpp>
#include <texture/texture-utils.h>
#include <camera/camera.hpp>
#include <camera/controllers/fly_camera_controller.hpp>

#include <glm/gtx/euler_angles.hpp>

#include <unordered_map>

struct Transform {
    glm::vec3 translation, rotation, scale;

    Transform(
            const glm::vec3& translation = {0,0,0},
            const glm::vec3& rotation = {0,0,0},
            const glm::vec3& scale = {1,1,1}
    ): translation(translation), rotation(rotation), scale(scale) {}

    glm::mat4 to_mat4() const {
        return glm::translate(glm::mat4(1.0f), translation) *
               glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

// This example demonstrates that a texture can be used to hold data that are not meant to be interpreted as colors.
class DisplacementApplication : public our::Application {

    our::ShaderProgram program;

    our::Mesh plane;

    std::unordered_map<std::string, GLuint> height_textures;
    std::string current_height_texture_name;
    GLuint top_texture = 0, bottom_texture = 0;

    GLuint height_sampler = 0, color_sampler = 0;

    GLenum polygon_mode = GL_FILL;

    Transform terrain;

    our::Camera camera;
    our::FlyCameraController camera_controller;

    glm::vec2 terrain_color_threshold = {0.3f, 0.4f};
    float texture_tiling = 32.0f;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Displacement", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        // This vertex shader moves the vertices up in the Y-direction based on the corresponding value in the height texture.
        program.attach("assets/shaders/ex24_displacement/terrain.vert", GL_VERTEX_SHADER);
        // This fragment shader blends between the colors sampled from 2 textures depending on the vertex height.
        program.attach("assets/shaders/ex24_displacement/terrain.frag", GL_FRAGMENT_SHADER);
        program.link();

        GLuint texture;
        // All the height textures are gray scale so we use a specific function we made that reads and stores only 1 channel per pixel.
        glGenTextures(1, &texture);
        our::texture_utils::loadImageGrayscale(texture, "assets/images/ex24_displacement/Heightmap_Default.png");
        height_textures["default"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImageGrayscale(texture, "assets/images/ex24_displacement/Heightmap_Billow.png");
        height_textures["billow"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImageGrayscale(texture, "assets/images/ex24_displacement/Heightmap_Island.png");
        height_textures["island"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImageGrayscale(texture, "assets/images/ex24_displacement/Heightmap_Mountain.png");
        height_textures["mountain"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImageGrayscale(texture, "assets/images/ex24_displacement/Heightmap_Plateau.png");
        height_textures["plateau"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImageGrayscale(texture, "assets/images/ex24_displacement/Heightmap_Rocky.png");
        height_textures["rocky"] = texture;

        current_height_texture_name = "default";

        // These textures will be used to color the terrain.
        glGenTextures(1, &top_texture);
        our::texture_utils::loadImage(top_texture, "assets/images/ex24_displacement/mntn_white_d.jpg");
        glGenTextures(1, &bottom_texture);
        our::texture_utils::loadImage(bottom_texture, "assets/images/ex24_displacement/grass_ground_d.jpg");

        our::mesh_utils::Plane(plane, {512, 512}, false);

        glGenSamplers(1, &height_sampler);
        // The height sampler is bound to unit 0 since we will later bind the height texture to unit 0.
        glBindSampler(0, height_sampler);

        // No need for mip maps for the height texture since we will sample it in the vertex shader so we won't be able to calculate the screen-space gradients.
        glSamplerParameteri(height_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(height_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // The height texture won't be repeated so no need to use repeat.
        glSamplerParameteri(height_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(height_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenSamplers(1, &color_sampler);
        // The color sampler is bound to units 1 & 2 since we will bind our 2 color textures to these units.
        glBindSampler(1, color_sampler);
        glBindSampler(2, color_sampler);

        // Here, we will use mip maps since they will be sampled in the fragment shader to get the fragment color so it would be nice to have trilinear filtering to enhance the visual quality.
        glSamplerParameteri(color_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(color_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // The color textures will be tiled, so we will use repeat wrap mode.
        glSamplerParameteri(color_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(color_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({120, 120, 120});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 2000.0f);

        camera_controller.initialize(this, &camera);
        camera_controller.setPositionSensitivity({10.0f, 10.0f, 10.0f} );

        terrain.scale = {512, 100, 512};

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glClearColor(0.88,0.65,0.15, 1);
    }

    void onDraw(double deltaTime) override {
        camera_controller.update(deltaTime);

        glUseProgram(program);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // First, we bind the height texture to unit 0 and set the "height_sampler" uniform to unit 0.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, height_textures[current_height_texture_name]);
        program.set("height_sampler", 0);

        // Then, we bind the top color texture to unit 1 and set the "terrain_top_sampler" uniform to unit 1.
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, top_texture);
        program.set("terrain_top_sampler", 1);

        // Finally, we bind the bottom color texture to unit 2 and set the "terrain_top_sampler" uniform to unit 2.
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, bottom_texture);
        program.set("terrain_bottom_sampler", 2);

        // Remember that we already bound the sampler in the "onInitialize" function so no need to do it here.

        glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);

        program.set("tint", glm::vec4(1,1,1,1));
        program.set("transform", camera.getVPMatrix() * terrain.to_mat4());
        program.set("texture_tiling", texture_tiling);
        program.set("terrain_color_threshold", terrain_color_threshold);

        plane.draw();
    }

    void onDestroy() override {
        program.destroy();
        glDeleteSamplers(1, &height_sampler);
        glDeleteSamplers(1, &color_sampler);
        plane.destroy();
        for(auto& [name, texture]: height_textures){
            glDeleteTextures(1, &texture);
        }
        height_textures.clear();
    }

    void onImmediateGui(ImGuiIO &io) override {

        ImGui::Begin("Controls");

        if(ImGui::BeginCombo("Heightmap", current_height_texture_name.c_str())){
            for(const auto& [name, texture]: height_textures){
                bool selected = current_height_texture_name == name;
                if(ImGui::Selectable(name.c_str(), selected))
                    current_height_texture_name = name;
                if(selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Image((void*)(intptr_t)height_textures[current_height_texture_name], {256, 256});

        ImGui::DragFloat("Texture Tiling", &texture_tiling);
        ImGui::DragFloat("Color Bottom Threshold", &terrain_color_threshold.x, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Color Top Threshold", &terrain_color_threshold.y, 0.01f, 0.0f, 1.0f);

        ImGui::Separator();

        our::OptionMapCombo("Polygon Mode", polygon_mode, our::gl_enum_options::polygon_modes);

        ImGui::End();


    }

};

int main(int argc, char** argv) {
    return DisplacementApplication().run();
}
