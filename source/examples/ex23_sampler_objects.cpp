#include <application.hpp>
#include <shader.hpp>
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


    Transform(
            const glm::vec4& tint = {1,1,1,1},
            const glm::vec3& translation = {0,0,0},
            const glm::vec3& rotation = {0,0,0},
            const glm::vec3& scale = {1,1,1},
            const std::optional<std::string>& mesh = std::nullopt,
            const std::string& texture = ""
    ): tint(tint), translation(translation), rotation(rotation), scale(scale), mesh(mesh) {}

    [[nodiscard]] glm::mat4 to_mat4() const {
        return glm::translate(glm::mat4(1.0f), translation) *
               glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

class SamplerObjectsApplication : public our::Application {

    our::ShaderProgram program;

    std::unordered_map<std::string, std::unique_ptr<our::Mesh>> meshes;

    std::unordered_map<std::string, GLuint> textures;

    GLuint sampler;

    GLenum magnification_filter = GL_LINEAR, minification_filter = GL_LINEAR_MIPMAP_LINEAR;
    GLenum wrap_s = GL_REPEAT, wrap_t = GL_REPEAT;
    glm::vec4 border_color = {1,1,1,1};
    GLfloat max_anisotropy = 1.0f;

    GLenum polygon_mode = GL_FILL;

    std::shared_ptr<Transform> root;

    our::Camera camera;
    our::FlyCameraController camera_controller;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Sampler Objects", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex22_texture_sampling/transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex22_texture_sampling/texture.frag", GL_FRAGMENT_SHADER);
        program.link();

        GLuint texture;

        glGenTextures(1, &texture);
        our::texture_utils::checkerBoard(texture, {256,256}, {128,128}, {255, 255, 255, 255}, {64, 64, 64, 255});
        textures["checkerboard"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/models/House/House.jpeg");
        textures["house"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/moon.jpg");
        textures["moon"] = texture;

        meshes["house"] = std::make_unique<our::Mesh>();
        our::mesh_utils::loadOBJ(*(meshes["house"]), "assets/models/House/House.obj");
        meshes["plane"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Plane(*(meshes["plane"]), {1, 1}, false, {0, 0, 0}, {1, 1}, {0, 0}, {100, 100});
        meshes["sphere"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Sphere(*(meshes["sphere"]), {32, 16}, false);

        glGenSamplers(1, &sampler);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        camera_controller.initialize(this, &camera);
        camera_controller.setFieldOfViewSensitivity(0.05f );

        std::ifstream file_in("assets/data/ex23_sampler_objects/scene.json");
        nlohmann::json json;
        file_in >> json;
        file_in.close();
        root = loadNode(json);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glClearColor(0.88,0.65,0.15, 1);
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindSampler(0, sampler);
        program.set("sampler", 0);

        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, magnification_filter);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, minification_filter);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, wrap_s);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, wrap_t);
        glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));
        glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy);

        glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);

        drawNode(root, camera.getVPMatrix());
    }

    void onDestroy() override {
        program.destroy();
        glDeleteSamplers(1, &sampler);
    }

    void onImmediateGui(ImGuiIO &io) override {

        ImGui::Begin("Controls");

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

        ImGui::Separator();

        our::OptionMapCombo("Polygon Mode", polygon_mode, our::gl_enum_options::polygon_modes);

        ImGui::End();


    }

};

int main(int argc, char** argv) {
    return SamplerObjectsApplication().run();
}
