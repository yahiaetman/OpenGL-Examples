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

// This struct will be used to define the material properties of an object
// Based on Phong Lighting Model, we need 3 components:
// 1- Ambient: which is light coming from equally from all directions such Sky Lights. It is a cheap approximation of indirect lighting.
//          The amount of ambient reflected by the material is specified by the variable "Ambient".
// 2- Diffuse: Which is the result of light falling on the surface getting scattered equally in all directions. We will approximate this using the Lambert formula.
//          The amount of diffuse reflected by the material is specified by the variable "Diffuse".
// 3- Specular: Which is the result of light reflecting off the surface in the reflection direction. We will use the Phong reflection model to approximate this.
//          The amount of specular reflected by the material is specified by the variable "Specular". The shininess (a.k.a. specular power) defines the surface smoothness/roughness.
struct Material {
    glm::vec3 diffuse, specular, ambient;
    float shininess;

    explicit Material(
            const glm::vec3& diffuse = {0,0,0},
            const glm::vec3& specular = {0,0,0},
            const glm::vec3& ambient = {0, 0, 0},
            float shininess = 1.0f
            ): diffuse(diffuse), specular(specular), ambient(ambient), shininess(shininess) {}
};

void from_json(const nlohmann::json& j, Material& m){
    m.diffuse = j.value<glm::vec3>("diffuse", {0.0f, 0.0f, 0.0f});
    m.specular = j.value<glm::vec3>("specular", {0.0f, 0.0f, 0.0f});
    m.ambient = j.value<glm::vec3>("ambient", {0.0f, 0.0f, 0.0f});
    m.shininess = j.value<float>("shininess", 1.0f);
}

// Now each object needs to define its material.
struct Transform {
    Material material;
    glm::vec3 translation, rotation, scale;
    std::optional<std::string> mesh;
    std::unordered_map<std::string, std::shared_ptr<Transform>> children;

    explicit Transform(
            const Material& material = Material(),
            const glm::vec3& translation = {0,0,0},
            const glm::vec3& rotation = {0,0,0},
            const glm::vec3& scale = {1,1,1},
            std::optional<std::string>  mesh = std::nullopt
    ): material(material), translation(translation), rotation(rotation), scale(scale), mesh(std::move(mesh)) {}

    [[nodiscard]] glm::mat4 to_mat4() const {
        return glm::translate(glm::mat4(1.0f), translation) *
               glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

// We will support 3 types of lights.
// 1- Directional Light: where we assume that the light rays are parallel. We use this to approximate sun light.
// 2- Point Light: where we assume that the light source is a single point that emits light in every direction. It can be used to approximate light bulbs.
// 3- Spot Light: where we assume that the light source is a single point that emits light in the direction of a cone. It can be used to approximate torches, highway light poles.
enum class LightType {
    DIRECTIONAL,
    POINT,
    SPOT
};

struct Light {
    // Here we define our light. First member specifies its type.
    LightType type;
    // We also define the color & intensity of the light for each component of the Phong model (Ambient, Diffuse, Specular).
    glm::vec3 diffuse, specular, ambient;
    glm::vec3 position; // Used for Point and Spot Lights only
    glm::vec3 direction; // Used for Directional and Spot Lights only
    // This affects how the light will dim out as we go further from the light.
    // The formula is light_received = light_emitted / (a*d^2 + b*d + c) where a, b, c are the quadratic, linear and constant factors respectively.
    struct {
        float constant, linear, quadratic;
    } attenuation; // Used for Point and Spot Lights only
    // This specifies the inner and outer cone of the spot light.
    // The light power is 0 outside the outer cone, the light power is full inside the inner cone.
    // The light power is interpolated in between the inner and outer cone.
    struct {
        float inner, outer;
    } spot_angle; // Used for Spot Lights only
};

// This example demonstrates how to draw a scene with shaders that approximate lights.
class LightApplication : public our::Application {

    // We will create a different shader program for each light type.
    std::unordered_map<LightType, our::ShaderProgram> programs;

    std::unordered_map<std::string, std::unique_ptr<our::Mesh>> meshes;

    std::shared_ptr<Transform> root;

    our::Camera camera;
    our::FlyCameraController camera_controller;

    Light light{};

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Light", {1280, 720}, false };
    }

    void onInitialize() override {
        // We will create a different shader program for each light type.
        programs[LightType::DIRECTIONAL].create();
        programs[LightType::DIRECTIONAL].attach("assets/shaders/ex29_light/light_transform.vert", GL_VERTEX_SHADER);
        programs[LightType::DIRECTIONAL].attach("assets/shaders/ex29_light/directional_light.frag", GL_FRAGMENT_SHADER);
        programs[LightType::DIRECTIONAL].link();
        programs[LightType::POINT].create();
        programs[LightType::POINT].attach("assets/shaders/ex29_light/light_transform.vert", GL_VERTEX_SHADER);
        programs[LightType::POINT].attach("assets/shaders/ex29_light/point_light.frag", GL_FRAGMENT_SHADER);
        programs[LightType::POINT].link();
        programs[LightType::SPOT].create();
        programs[LightType::SPOT].attach("assets/shaders/ex29_light/light_transform.vert", GL_VERTEX_SHADER);
        programs[LightType::SPOT].attach("assets/shaders/ex29_light/spot_light.frag", GL_FRAGMENT_SHADER);
        programs[LightType::SPOT].link();

        meshes["suzanne"] = std::make_unique<our::Mesh>();
        our::mesh_utils::loadOBJ(*(meshes["suzanne"]), "assets/models/Suzanne/Suzanne.obj");
        meshes["plane"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Plane(*(meshes["plane"]), {1, 1}, false, {0, 0, 0}, {1, 1}, {0, 0}, {100, 100});
        meshes["sphere"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Sphere(*(meshes["sphere"]), {32, 16}, false);


        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        camera_controller.initialize(this, &camera);
        camera_controller.setFieldOfViewSensitivity(0.05f );

        std::ifstream file_in("assets/data/ex29_light/scene.json");
        nlohmann::json json;
        file_in >> json;
        file_in.close();
        root = loadNode(json);

        light.type = LightType::DIRECTIONAL;
        light.diffuse = {1,1,1};
        light.specular = {1,1,1};
        light.ambient = {0.1f, 0.1f, 0.1f};
        light.direction = {-1, -1, -1};
        light.position = {0, 1, 2};
        light.attenuation = {0, 0, 1};
        light.spot_angle = {glm::pi<float>()/4, glm::pi<float>()/2};

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glClearColor(0.88,0.65,0.15, 1);
    }

    std::shared_ptr<Transform> loadNode(const nlohmann::json& json){
        auto node = std::make_shared<Transform>(
                json.value<Material>("material", Material()),
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

    void drawNode(const std::shared_ptr<Transform>& node, const glm::mat4& parent_transform_matrix, our::ShaderProgram& program){
        glm::mat4 transform_matrix = parent_transform_matrix * node->to_mat4();
        if(node->mesh.has_value()){
            if(auto mesh_it = meshes.find(node->mesh.value()); mesh_it != meshes.end()) {
                // For each model, we will send the model matrix, model inverse transpose and material properties.
                program.set("object_to_world", transform_matrix);
                program.set("object_to_world_inv_transpose", glm::inverse(transform_matrix), true);
                program.set("material.diffuse", node->material.diffuse);
                program.set("material.specular", node->material.specular);
                program.set("material.ambient", node->material.ambient);
                program.set("material.shininess", node->material.shininess);
                mesh_it->second->draw();
            }
        }
        for(auto& [name, child]: node->children){
            drawNode(child, transform_matrix, program);
        }
    }

    void onDraw(double deltaTime) override {
        camera_controller.update(deltaTime);

        // We will pick the shader based on the light type
        auto& program = programs[light.type];

        glUseProgram(program);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // From the camera, we will send the camera position and view-projection matrix.
        program.set("camera_position", camera.getEyePosition());
        program.set("view_projection", camera.getVPMatrix());

        // Then we will send the light properties
        program.set("light.diffuse", light.diffuse);
        program.set("light.specular", light.specular);
        program.set("light.ambient", light.ambient);
        // Some properties are only available in some light types
        switch(light.type){
            case LightType::DIRECTIONAL:
                program.set("light.direction", glm::normalize(light.direction));
                break;
            case LightType::POINT:
                program.set("light.position", light.position);
                program.set("light.attenuation_constant", light.attenuation.constant);
                program.set("light.attenuation_linear", light.attenuation.linear);
                program.set("light.attenuation_quadratic", light.attenuation.quadratic);
                break;
            case LightType::SPOT:
                program.set("light.position", light.position);
                program.set("light.direction", glm::normalize(light.direction));
                program.set("light.attenuation_constant", light.attenuation.constant);
                program.set("light.attenuation_linear", light.attenuation.linear);
                program.set("light.attenuation_quadratic", light.attenuation.quadratic);
                program.set("light.inner_angle", light.spot_angle.inner);
                program.set("light.outer_angle", light.spot_angle.outer);
                break;
        }

        // Since we already sent the view-projection matrix already, we will only send the model matrices from the drawNode function.
        // That's why we are now sending an identity matrix as the parent transform matrix.
        drawNode(root, glm::mat4(1.0f), program);
    }

    void onDestroy() override {
        for(auto& [type, program]: programs){
            program.destroy();
        }
        programs.clear();
        for(auto& [name, mesh]: meshes){
            mesh->destroy();
        }
        meshes.clear();
    }

    void displayNodeGui(const std::shared_ptr<Transform>& node, const std::string& node_name){
        if(ImGui::TreeNode(node_name.c_str())){
            if(node->mesh.has_value()) {
                our::PairIteratorCombo("Mesh", node->mesh.value(), meshes.begin(), meshes.end());
                ImGui::ColorEdit3("Diffuse", glm::value_ptr(node->material.diffuse), ImGuiColorEditFlags_HDR);
                ImGui::ColorEdit3("Specular", glm::value_ptr(node->material.specular), ImGuiColorEditFlags_HDR);
                ImGui::ColorEdit3("Ambient", glm::value_ptr(node->material.ambient), ImGuiColorEditFlags_HDR);
                ImGui::DragFloat("Shininess", &(node->material.shininess), 0.1f, glm::epsilon<float>(), 1000000.0f);
            }
            ImGui::DragFloat3("Translation", glm::value_ptr(node->translation), 0.1f);
            ImGui::DragFloat3("Rotation", glm::value_ptr(node->rotation), 0.01f);
            ImGui::DragFloat3("Scale", glm::value_ptr(node->scale), 0.1f);
            for(auto& [name, child] : node->children){
                displayNodeGui(child, name);
            }
            ImGui::TreePop();
        }
    }

    void onImmediateGui(ImGuiIO &io) override {
        static const std::unordered_map<LightType, const char*> light_type_names = {
                {LightType::DIRECTIONAL, "Directional"},
                {LightType::POINT, "Point"},
                {LightType::SPOT, "Spot"}
        };

        ImGui::Begin("Light");

        if(ImGui::BeginCombo("Type", light_type_names.at(light.type))){
            for(auto& [type, name] : light_type_names){
                bool selected = light.type == type;
                if(ImGui::Selectable(name, selected))
                    light.type = type;
                if(selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::ColorEdit3("Diffuse", glm::value_ptr(light.diffuse), ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit3("Specular", glm::value_ptr(light.specular), ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit3("Ambient", glm::value_ptr(light.ambient), ImGuiColorEditFlags_HDR);

        switch(light.type){
            case LightType::DIRECTIONAL:
                ImGui::DragFloat3("Direction", glm::value_ptr(light.direction), 0.1f);
                break;
            case LightType::POINT:
                ImGui::DragFloat3("Position", glm::value_ptr(light.position), 0.1f);
                ImGui::Separator();
                ImGui::DragFloat("Constant Attenuation", &light.attenuation.constant, 0.1f);
                ImGui::DragFloat("Linear Attenuation", &light.attenuation.linear, 0.1f);
                ImGui::DragFloat("Quadratic Attenuation", &light.attenuation.quadratic, 0.1f);
                break;
            case LightType::SPOT:
                ImGui::DragFloat3("Direction", glm::value_ptr(light.direction), 0.1f);
                ImGui::DragFloat3("Position", glm::value_ptr(light.position), 0.1f);
                ImGui::Separator();
                ImGui::DragFloat("Constant Attenuation", &light.attenuation.constant, 0.1f);
                ImGui::DragFloat("Linear Attenuation", &light.attenuation.linear, 0.1f);
                ImGui::DragFloat("Quadratic Attenuation", &light.attenuation.quadratic, 0.1f);
                ImGui::Separator();
                ImGui::DragFloat("Inner Spot Angle", &light.spot_angle.inner, 0.1f, 0.0f, glm::two_pi<float>());
                ImGui::DragFloat("Outer Spot Angle", &light.spot_angle.outer, 0.1f, 0.0f, glm::two_pi<float>());
                break;
        }

        ImGui::End();

        ImGui::Begin("Scene");

        displayNodeGui(root, "root");

        ImGui::End();

    }

};

int main(int argc, char** argv) {
    return LightApplication().run();
}
