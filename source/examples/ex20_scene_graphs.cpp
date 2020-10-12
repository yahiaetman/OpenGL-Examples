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
#include <unordered_map>

#include <glm/gtx/string_cast.hpp>

namespace glm {
    template<length_t L, typename T, qualifier Q>
    void from_json(const nlohmann::json& j, vec<L, T, Q>& v){
        for(length_t index = 0; index < L; ++index)
            v[index] = j[index].get<T>();
    }
}

struct Transform {
    std::string name;
    glm::vec4 tint;
    glm::vec3 translation, rotation, scale;
    std::weak_ptr<our::Mesh> mesh;
    std::vector<std::shared_ptr<Transform>> children;


    Transform(
            const std::string& name = "Object",
            const glm::vec4& tint = {1,1,1,1},
            const glm::vec3& translation = {0,0,0},
            const glm::vec3& rotation = {0,0,0},
            const glm::vec3& scale = {1,1,1},
            std::weak_ptr<our::Mesh> mesh = std::weak_ptr<our::Mesh>()
                    ): name(name), tint(tint), translation(translation), rotation(rotation), scale(scale), mesh(mesh) {}

    glm::mat4 to_mat4() const {
        return glm::translate(glm::mat4(1.0f), translation) *
                glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
                glm::scale(glm::mat4(1.0f), scale);
    }
};

class SceneGraphApplication : public our::Application {

    our::ShaderProgram program;
    std::unordered_map<std::string, std::shared_ptr<our::Mesh>> meshes;

    std::shared_ptr<Transform> root;

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

        meshes["Cuboid"] = std::make_shared<our::Mesh>();
        our::mesh_utils::Cuboid(*(meshes["Cuboid"]), true);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        controller.initialize(this, &camera);

        loadSceneGraph("assets/data/ex20_scene_graphs/scene.json");

        glClearColor(0, 0, 0, 0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glClearColor(0, 0, 0, 1);
    }

    std::shared_ptr<Transform> loadNode(const nlohmann::json& json){
        auto node = std::make_shared<Transform>(
                json.value("name", ""),
                json.value<glm::vec4>("tint", {1,1,1,1}),
                json.value<glm::vec3>("translation", {0, 0, 0}),
                json.value<glm::vec3>("rotation", {0, 0, 0}),
                json.value<glm::vec3>("scale", {1, 1, 1})
        );
        if(json.contains("mesh")){
            auto mesh_name = json["mesh"].get<std::string>();
            auto it = meshes.find(mesh_name);
            if(it != meshes.end())
                node->mesh = it->second;
        }
        if(json.contains("children")){
            for(auto& child: json["children"]){
                node->children.push_back(loadNode(child));
            }
        }
        return node;
    }

    void loadSceneGraph(const std::string& scene_file){
        std::ifstream file_in(scene_file);
        nlohmann::json json;
        file_in >> json;
        file_in.close();

        root = loadNode(json);
    }

    void drawNode(const std::shared_ptr<Transform>& node, const glm::mat4& parent_transform_matrix){
        glm::mat4 transform_matrix = parent_transform_matrix * node->to_mat4();
        if(!node->mesh.expired()){
            //std::cout << "Node: " << node->name << " -> " << glm::to_string(transform_matrix) << std::endl;
            program.set("tint", node->tint);
            program.set("transform", transform_matrix);
            node->mesh.lock()->draw();
        }
        for(auto& child: node->children){
            drawNode(child, transform_matrix);
        }
    }

    void onDraw(double deltaTime) override {
        controller.update(deltaTime);

        glUseProgram(program);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawNode(root, camera.getVPMatrix());

    }

    void onDestroy() override {
        program.destroy();
        for(auto& [name, mesh] : meshes){
            mesh->destroy();
        }
        meshes.clear();
    }

};

int main(int argc, char** argv) {
    return SceneGraphApplication().run();
}
