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

class FrameBufferApplication : public our::Application {

    our::ShaderProgram program;

    std::unordered_map<std::string, std::unique_ptr<our::Mesh>> meshes;

    std::unordered_map<std::string, GLuint> textures;

    GLuint sampler;

    std::shared_ptr<Transform> root, internal_root;

    our::Camera camera, internal_camera;
    our::FlyCameraController camera_controller, internal_camera_controller;

    bool control_internal_camera = false;

    GLuint frame_buffer;

    const glm::ivec2 rt_size = {512, 512};

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Frame Buffer", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex22_texture_sampling/transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex22_texture_sampling/texture.frag", GL_FRAGMENT_SHADER);
        program.link();

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

        GLuint rt_levels = glm::floor(glm::log2(glm::max<float>(rt_size.x, rt_size.y))) + 1;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, rt_levels, GL_RGBA8, rt_size.x, rt_size.y);
        textures["color_rt"] = texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, rt_size.x, rt_size.y);
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
        glBindSampler(0, sampler);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>() / 2, static_cast<float>(width) / height, 0.1f, 100.0f);

        camera_controller.initialize(this, &camera);

        internal_camera.setEyePosition({10, 10, 10});
        internal_camera.setTarget({0, 0, 0});
        internal_camera.setUp({0, 1, 0});
        internal_camera.setupPerspective(glm::pi<float>() / 2, static_cast<float>(rt_size.x) / rt_size.y, 0.1f, 100.0f);

        internal_camera_controller.initialize(this, &internal_camera);

        root = loadSceneGraph("assets/data/ex26_frame_buffer/external.json");
        internal_root = loadSceneGraph("assets/data/ex23_sampler_objects/scene.json");

        glGenFramebuffers(1, &frame_buffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures["color_rt"], 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures["depth_rt"], 0);

        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            std::cerr << "Frame buffer is incomplete" << std::endl;
        }

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
        if(control_internal_camera)
            internal_camera_controller.update(deltaTime);
        else
            camera_controller.update(deltaTime);

        root->children["moon-axis"]->children["moon"]->rotation.y += deltaTime;
        internal_root->children["moon-axis"]->children["moon"]->rotation.y += deltaTime;

        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        program.set("sampler", 0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer);
        glViewport(0, 0, rt_size.x, rt_size.y);

        glClearColor(0.88,0.65,0.15, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawNode(internal_root, internal_camera.getVPMatrix());

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glBindTexture(GL_TEXTURE_2D, textures["color_rt"]);
        glGenerateMipmap(GL_TEXTURE_2D);

        glClearColor(0.05,0.1,0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawNode(root, camera.getVPMatrix());
    }

    void onDestroy() override {
        program.destroy();
        glDeleteSamplers(1, &sampler);
        glDeleteFramebuffers(1, &frame_buffer);
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

        ImGui::Checkbox("Control Internal Camera", &control_internal_camera);

        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return FrameBufferApplication().run();
}
