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

/******************************     Explaining the JSON     ****************************************/
/*  SIMPLE.JSON
{
  "name": "root",
  "mesh": "cube",
  "children": {
    "child-1": {
      "translation": [-2, 2, 0],
      "scale": [0.5, 0.5, 0.5],
      "mesh": "cube",
      "children": {
        "grand-child-1": {
          "translation": [0, 2, -2],
          "scale": [0.5, 0.5, 0.5],
          "mesh": "cube"
        },
        "grand-child-2": {
          "translation": [0, 2, 2],
          "scale": [0.5, 0.5, 0.5],
          "mesh": "cube"
        }
      }
    },
    "child-2": {
      "translation": [2, 2, 0],
      "scale": [0.5, 0.5, 0.5],
      "mesh": "cube",
      "children": {
        "grand-child-1": {
          "translation": [0, 2, -2],
          "scale": [0.5, 0.5, 0.5],
          "mesh": "cube"
        },
        "grand-child-2": {
          "translation": [0, 2, 2],
          "scale": [0.5, 0.5, 0.5],
          "mesh": "cube"
        }
      }
    }
  }
}
*/

// The name of the root is "root" it has cube mesh.
// It has the root has 2 children (child-1) and (child-2).
// child-1 has transform data: translation, scale. It also has a mesh (cube mesh).

// child-1 has 2 children (grand-child-1) and (grand-child-2).
// grand-child-1 also has transform data: translation, scale. It also has a mesh (cube mesh).

// Notice that the transform data (translation,rotation,scale) propogates from parent to child.

/***************************************************************************************************/


// The struct holds the Transform data.
struct Transform {
    // This vec4 is multiplied by the original color.
    glm::vec4 tint;
    // Vectors to transform data. 
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

    // This unordered map keep track of all our meshes and saves them by name.
    // meshes["cube"] => contains the mesh data of a cube.
    std::unordered_map<std::string, std::unique_ptr<our::Mesh>> meshes;

    // This unordered map keep track of all our meshes and saves them by name.
    std::unordered_map<std::string, std::shared_ptr<Transform>> roots;

    // This value stores the name of the root of the whole scene.
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

        // Create meshes for cube, rod (cube shifted by 0.5 in z), and a sphere.
        meshes["cube"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Cuboid(*(meshes["cube"]), true);
        meshes["rod"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Cuboid(*(meshes["rod"]), true, {0, 0, 0.5});
        meshes["sphere"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Sphere(*(meshes["sphere"]), {32, 16}, true);

        // Set the camera data.
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        controller.initialize(this, &camera);

        // Reading each file saving only the parent of the whole scene in "root".
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

    // Loading node happens 
    std::shared_ptr<Transform> loadNode(const nlohmann::json& json){
        // First read from the JSON the values of tint,translation,rotation,scale from json.
        // in Json:
        ///      "translation": [0, 2, -2]
        // then translation = 0, 2, -2
        auto node = std::make_shared<Transform>(
                json.value<glm::vec4>("tint", {1,1,1,1}),
                json.value<glm::vec3>("translation", {0, 0, 0}),
                json.value<glm::vec3>("rotation", {0, 0, 0}),
                json.value<glm::vec3>("scale", {1, 1, 1})
        );
        // If it contains mesh name set it.
        if(json.contains("mesh")){
            node->mesh = json["mesh"].get<std::string>();
        }
        // If it contains children nodes set them.
        // Notice that this function is recursive, it sets all the children data to the set of children,
        // also the data of each child.
        if(json.contains("children")){
            for(auto& [name, child]: json["children"].items()){
                node->children[name] = loadNode(child);
            }
        }
        return node;
    }

    // Read the data of the JSON file and send it to the "loadNode()" to be processed.
    std::shared_ptr<Transform> loadSceneGraph(const std::string& scene_file){
        std::ifstream file_in(scene_file);
        nlohmann::json json;
        file_in >> json;
        file_in.close();

        return loadNode(json);
    }

    // This function draws the models in a recursive manner.
    // It starts with the parent node and draw each child,
    // passing the transform matrix to the children to
    // cassacade the transformation effect.
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
