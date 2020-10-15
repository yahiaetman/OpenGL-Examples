#include <application.hpp>
#include <shader.hpp>
#include <utility>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <mesh/mesh-utils.hpp>
#include <texture/texture-utils.h>
#include <camera/camera.hpp>
#include <camera/controllers/fly_camera_controller.hpp>

#include <glm/gtx/euler_angles.hpp>

#include <json/json.hpp>

#include <fstream>
#include <unordered_map>

namespace glm {
    template<length_t L, typename T, qualifier Q>
    void from_json(const nlohmann::json& j, vec<L, T, Q>& v){
        for(length_t index = 0; index < L; ++index)
            v[index] = j[index].get<T>();
    }
}

struct Transform {
    glm::vec4 tint;
    glm::vec3 translation, rotation, scale;
    std::optional<std::string> mesh;
    std::string texture;
    std::unordered_map<std::string, std::shared_ptr<Transform>> children;


    explicit Transform(
            const glm::vec4& tint = {1,1,1,1},
            const glm::vec3& translation = {0,0,0},
            const glm::vec3& rotation = {0,0,0},
            const glm::vec3& scale = {1,1,1},
            std::optional<std::string>  mesh = std::nullopt,
            std::string  texture = ""
    ): tint(tint), translation(translation), rotation(rotation), scale(scale), mesh(std::move(mesh)), texture(std::move(texture)) {}

    [[nodiscard]] glm::mat4 to_mat4() const {
        return glm::translate(glm::mat4(1.0f), translation) *
               glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

enum class PostProcessingEffectTypes: int {
    BLIT,
    DISTORTION,
    FOG,
    ENUM_SIZE
};

const char* const PostProcessingEffectTypeNames[] = {
        "Blit",
        "Distortion",
        "Fog"
};

class PostProcessingApplication : public our::Application {

    our::ShaderProgram program;

    std::unordered_map<std::string, std::unique_ptr<our::Mesh>> meshes;

    std::unordered_map<std::string, GLuint> textures;

    GLuint sampler, screen_color_sampler;

    std::shared_ptr<Transform> root;

    our::Camera camera;
    our::FlyCameraController camera_controller;

    GLuint frame_buffer, fullscreen_vertex_array;

    struct {
        our::ShaderProgram effect_program;
    } blit;
    struct {
        our::ShaderProgram effect_program;
        float distortion_power = 0.05f;
    } distortion;
    struct {
        our::ShaderProgram effect_program;
        glm::vec3 fog_color = {0.75, 0.5, 0.25};
        float fog_power = 1.0f;
        float fog_distance = 10.0f;
    } fog;
    PostProcessingEffectTypes current_effect = PostProcessingEffectTypes::DISTORTION;


    our::WindowConfiguration getWindowConfiguration() override {
        return { "Post-Processing", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex22_texture_sampling/transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex22_texture_sampling/texture.frag", GL_FRAGMENT_SHADER);
        program.link();
        blit.effect_program.create();
        blit.effect_program.attach("assets/shaders/ex27_postprocessing/fullscreen_triangle.vert", GL_VERTEX_SHADER);
        blit.effect_program.attach("assets/shaders/ex27_postprocessing/blit.frag", GL_FRAGMENT_SHADER);
        blit.effect_program.link();
        distortion.effect_program.create();
        distortion.effect_program.attach("assets/shaders/ex27_postprocessing/fullscreen_triangle.vert", GL_VERTEX_SHADER);
        distortion.effect_program.attach("assets/shaders/ex27_postprocessing/distortion.frag", GL_FRAGMENT_SHADER);
        distortion.effect_program.link();
        fog.effect_program.create();
        fog.effect_program.attach("assets/shaders/ex27_postprocessing/fullscreen_triangle.vert", GL_VERTEX_SHADER);
        fog.effect_program.attach("assets/shaders/ex27_postprocessing/fog.frag", GL_FRAGMENT_SHADER);
        fog.effect_program.link();

        GLuint texture;

        glGenTextures(1, &texture);
        our::texture_utils::checkerBoard(texture, {256, 256}, {128, 128}, {255, 255, 255, 255}, {16, 16, 16, 255});
        textures["checkerboard"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/models/House/House.jpeg");
        textures["house"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/moon.jpg");
        textures["moon"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/ex27_postprocessing/water-normal.png");
        textures["water-normal"] = texture;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        GLuint rt_levels = glm::floor(glm::log2(glm::max<float>(width, height))) + 1;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, rt_levels, GL_RGBA8, width, height);
        textures["color_rt"] = texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, width, height);
        textures["depth_rt"] = texture;

        meshes["house"] = std::make_unique<our::Mesh>();
        our::mesh_utils::loadOBJ(*(meshes["house"]), "assets/models/House/House.obj");
        meshes["plane"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Plane(*(meshes["plane"]), {1, 1}, false, {0, 0, 0}, {1, 1}, {0, 0}, {100, 100});
        meshes["sphere"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Sphere(*(meshes["sphere"]), {32, 16}, false);
        meshes["cube"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Cuboid(*(meshes["cube"]));

        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glGenSamplers(1, &screen_color_sampler);
        glSamplerParameteri(screen_color_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(screen_color_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(screen_color_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glSamplerParameteri(screen_color_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>() / 2, static_cast<float>(width) / height, 0.1f, 100.0f);

        camera_controller.initialize(this, &camera);

        root = loadSceneGraph("assets/data/ex23_sampler_objects/scene.json");

        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures["color_rt"], 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures["depth_rt"], 0);

        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            std::cerr << "Frame buffer is incomplete" << std::endl;
        }

        glGenVertexArrays(1, &fullscreen_vertex_array);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    std::shared_ptr<Transform> loadNode(const nlohmann::json& json){
        auto node = std::make_shared<Transform>(
                json.value<glm::vec4>("tint", {1,1,1,1}),
                json.value<glm::vec3>("translation", {0, 0, 0}),
                json.value<glm::vec3>("rotation", {0, 0, 0}),
                json.value<glm::vec3>("scale", {1, 1, 1})
        );
        if(json.contains("mesh")){
            node->mesh = json["mesh"].get<std::string>();
        }
        if(json.contains("texture")){
            node->texture = json["texture"].get<std::string>();
        }
        if(json.contains("children")){
            for(auto& [name, child]: json["children"].items()){
                node->children[name] = loadNode(child);
            }
        }
        return node;
    }

    std::shared_ptr<Transform> loadSceneGraph(const std::string& filename){
        std::ifstream file_in(filename);
        nlohmann::json json;
        file_in >> json;
        file_in.close();
        return loadNode(json);
    }

    void drawNode(const std::shared_ptr<Transform>& node, const glm::mat4& parent_transform_matrix){
        glm::mat4 transform_matrix = parent_transform_matrix * node->to_mat4();
        if(node->mesh.has_value()){
            if(auto mesh_it = meshes.find(node->mesh.value()); mesh_it != meshes.end()) {
                GLuint texture = 0;
                if(auto tex_it = textures.find(node->texture); tex_it != textures.end())
                    texture = tex_it->second;
                glBindTexture(GL_TEXTURE_2D, texture);
                program.set("tint", node->tint);
                program.set("transform", transform_matrix);
                mesh_it->second->draw();
            }
        }
        for(auto& [name, child]: node->children){
            drawNode(child, transform_matrix);
        }
    }

    void onDraw(double deltaTime) override {
        camera_controller.update(deltaTime);

        root->children["moon-axis"]->children["moon"]->rotation.y += deltaTime;

        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        glBindSampler(0, sampler);
        program.set("sampler", 0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer);

        glClearColor(0.88,0.65,0.15, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawNode(root, camera.getVPMatrix());

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glDisable(GL_DEPTH_TEST);

        switch (current_effect) {
            case PostProcessingEffectTypes::BLIT:
                glUseProgram(blit.effect_program);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures["color_rt"]);
                glGenerateMipmap(GL_TEXTURE_2D);
                blit.effect_program.set("color_sampler", 0);

                break;
            case PostProcessingEffectTypes::DISTORTION:
                glUseProgram(distortion.effect_program);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures["color_rt"]);
                glBindSampler(0, screen_color_sampler);
                glGenerateMipmap(GL_TEXTURE_2D);
                distortion.effect_program.set("color_sampler", 0);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, textures["water-normal"]);
                glBindSampler(1, sampler);
                distortion.effect_program.set("distortion_sampler", 1);

                distortion.effect_program.set("distortion_power", distortion.distortion_power);
                break;
            case PostProcessingEffectTypes::FOG:
                glUseProgram(fog.effect_program);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textures["color_rt"]);
                glBindSampler(0, screen_color_sampler);
                glGenerateMipmap(GL_TEXTURE_2D);
                fog.effect_program.set("color_sampler", 0);

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, textures["depth_rt"]);
                glBindSampler(1, screen_color_sampler);
                fog.effect_program.set("depth_sampler", 1);

                fog.effect_program.set("inverse_projection", glm::inverse(camera.getProjectionMatrix()));
                fog.effect_program.set("fog_color", fog.fog_color);
                fog.effect_program.set("fog_power", fog.fog_power);
                fog.effect_program.set("fog_exponent", 1.0f / fog.fog_distance);
                break;
            default:
                std::cerr << "Invalid Effect Option" << std::endl;
        }

        glBindVertexArray(fullscreen_vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);

    }

    void onDestroy() override {
        program.destroy();
        blit.effect_program.destroy();
        distortion.effect_program.destroy();
        fog.effect_program.destroy();
        glDeleteSamplers(1, &sampler);
        glDeleteSamplers(1, &screen_color_sampler);
        glDeleteFramebuffers(1, &frame_buffer);
        glDeleteVertexArrays(1, &fullscreen_vertex_array);
        for(auto& [name, texture]: textures){
            glDeleteTextures(1, &texture);
        }
        textures.clear();
        for(auto& [name, mesh]: meshes){
            mesh->destroy();
        }
        meshes.clear();
    }

    void onImmediateGui(ImGuiIO &io) override {
        ImGui::Begin("Controls");

        if(ImGui::BeginCombo("Effect", PostProcessingEffectTypeNames[static_cast<int>(current_effect)])){
            for(int index = 0; index < static_cast<int>(PostProcessingEffectTypes::ENUM_SIZE); ++index){
                bool selected = static_cast<int>(current_effect) == index;
                if(ImGui::Selectable(PostProcessingEffectTypeNames[index], selected))
                    current_effect = static_cast<PostProcessingEffectTypes>(index);
                if(selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        switch(current_effect){
            case PostProcessingEffectTypes::BLIT:
                break;
            case PostProcessingEffectTypes::DISTORTION:
                ImGui::DragFloat("Distortion Power", &distortion.distortion_power, 0.01f);
                break;
            case PostProcessingEffectTypes::FOG:
                ImGui::DragFloat("Fog Power", &fog.fog_power, 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("Fog Distance", &fog.fog_distance, 0.1f, 0.0f, 1000.0f);
                ImGui::ColorEdit3("Fog Color", glm::value_ptr(fog.fog_color));
                break;
            default:
                break;
        }

        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return PostProcessingApplication().run();
}
