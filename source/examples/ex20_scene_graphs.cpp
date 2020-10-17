#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <mesh/mesh-utils.hpp>
#include <camera/camera.hpp>
#include <camera/controllers/fly_camera_controller.hpp>

#include <glm/gtx/euler_angles.hpp>

#include <json/json.hpp>

#include <fstream>
#include <string>
#include <unordered_map>
#include <optional>

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
    std::unordered_map<std::string, std::shared_ptr<Transform>> children;


    Transform(
            const glm::vec4& tint = {1,1,1,1},
            const glm::vec3& translation = {0,0,0},
            const glm::vec3& rotation = {0,0,0},
            const glm::vec3& scale = {1,1,1},
            const std::optional<std::string>& mesh = std::nullopt
                    ): tint(tint), translation(translation), rotation(rotation), scale(scale), mesh(mesh) {}

    [[nodiscard]] glm::mat4 to_mat4() const {
        return glm::translate(glm::mat4(1.0f), translation) *
                glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
                glm::scale(glm::mat4(1.0f), scale);
    }
};

class SceneGraphApplication : public our::Application {

    our::ShaderProgram program;
    std::unordered_map<std::string, std::unique_ptr<our::Mesh>> meshes;

    std::unordered_map<std::string, std::shared_ptr<Transform>> roots;
    std::string current_root_name;

    our::Camera camera;
    our::FlyCameraController controller;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Scene Graphs", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex11_transformation/transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex11_transformation/tint.frag", GL_FRAGMENT_SHADER);
        program.link();

        meshes["cube"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Cuboid(*(meshes["cube"]), true);
        meshes["rod"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Cuboid(*(meshes["rod"]), true, {0, 0, 0.5});
        meshes["sphere"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Sphere(*(meshes["sphere"]), {32, 16}, true);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        controller.initialize(this, &camera);

        roots["simple"] = loadSceneGraph("assets/data/ex20_scene_graphs/simple.json");
        roots["solar-system"] = loadSceneGraph("assets/data/ex20_scene_graphs/solar-system.json");
        roots["human"] = loadSceneGraph("assets/data/ex20_scene_graphs/human.json");
        current_root_name = "simple";

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glClearColor(0, 0, 0, 1);
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
        if(json.contains("children")){
            for(auto& [name, child]: json["children"].items()){
                node->children[name] = loadNode(child);
            }
        }
        return node;
    }

    std::shared_ptr<Transform> loadSceneGraph(const std::string& scene_file){
        std::ifstream file_in(scene_file);
        nlohmann::json json;
        file_in >> json;
        file_in.close();

        return loadNode(json);
    }

    void drawNode(const std::shared_ptr<Transform>& node, const glm::mat4& parent_transform_matrix){
        glm::mat4 transform_matrix = parent_transform_matrix * node->to_mat4();
        if(node->mesh.has_value()){
            auto it = meshes.find(node->mesh.value());
            if(it != meshes.end()) {
                program.set("tint", node->tint);
                program.set("transform", transform_matrix);
                it->second->draw();
            }
        }
        for(auto& [name, child]: node->children){
            drawNode(child, transform_matrix);
        }
    }

    void onDraw(double deltaTime) override {
        controller.update(deltaTime);

        glUseProgram(program);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawNode(roots[current_root_name], camera.getVPMatrix());

    }

    void onDestroy() override {
        program.destroy();
        for(auto& [name, mesh] : meshes){
            mesh->destroy();
        }
        meshes.clear();
    }

    void displayNodeGui(const std::shared_ptr<Transform>& node, const std::string& node_name){
        if(ImGui::TreeNode(node_name.c_str())){
            if(node->mesh.has_value()) {
                our::PairIteratorCombo("Mesh", node->mesh.value(), meshes.begin(), meshes.end());
                ImGui::ColorEdit4("Tint", glm::value_ptr(node->tint));
            }
            ImGui::DragFloat3("Translation", glm::value_ptr(node->translation), 0.1f);
            ImGui::DragFloat3("Rotation", glm::value_ptr(node->rotation), 0.01f);
            ImGui::DragFloat3("Scale", glm::value_ptr(node->scale), 0.1f);
            for(auto& [name, child]: node->children){
                displayNodeGui(child, name);
            }
            ImGui::TreePop();
        }
    }

    void onImmediateGui(ImGuiIO &io) override {
        ImGui::Begin("Scene Graph");

        if(ImGui::BeginCombo("Scene", current_root_name.c_str())){
            for(auto& [name, root] : roots){
                bool selected = current_root_name == name;
                if(ImGui::Selectable(name.c_str(), selected))
                    current_root_name = name;
                if(selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        displayNodeGui(roots[current_root_name], current_root_name);
        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return SceneGraphApplication().run();
}
