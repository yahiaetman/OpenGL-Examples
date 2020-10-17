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
#include <algorithm>
#include <cctype>
#include <string>

namespace glm {
    template<length_t L, typename T, qualifier Q>
    void from_json(const nlohmann::json& j, vec<L, T, Q>& v){
        for(length_t index = 0; index < L; ++index)
            v[index] = j[index].get<T>();
    }
}

struct Material {
    std::string albedo_map, specular_map, roughness_map, ambient_occlusion_map, emissive_map;
    glm::vec3 albedo_tint, specular_tint, emissive_tint;
    glm::vec2 roughness_range;
};

void from_json(const nlohmann::json& j, Material& m){
    m.albedo_map = j.value<std::string>("albedo_map", "white");
    m.albedo_tint = j.value<glm::vec3>("albedo_tint", {1.0f, 1.0f, 1.0f});
    m.specular_map = j.value<std::string>("specular_map", "black");
    m.specular_tint = j.value<glm::vec3>("specular_tint", {1.0f, 1.0f, 1.0f});
    m.roughness_map = j.value<std::string>("roughness_map", "white");
    m.roughness_range = j.value<glm::vec2>("roughness_scale", {0.0f, 1.0f});
    m.ambient_occlusion_map = j.value<std::string>("ambient_occlusion_map", "white");
    m.emissive_map = j.value<std::string>("emissive_map", "black");
    m.emissive_tint = j.value<glm::vec3>("emissive_tint", {1.0f, 1.0f, 1.0f});

}

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

enum class LightType {
    DIRECTIONAL,
    POINT,
    SPOT
};

struct Light {
    LightType type;
    bool enabled;
    glm::vec3 color;
    glm::vec3 position; // Used for Point and Spot Lights only
    glm::vec3 direction; // Used for Directional and Spot Lights only
    struct {
        float constant, linear, quadratic;
    } attenuation; // Used for Point and Spot Lights only
    struct {
        float inner, outer;
    } spot_angle; // Used for Spot Lights only
};

void from_json(const nlohmann::json& j, Light& l){
    std::string type_name = j.value("type", "point");
    std::transform(type_name.begin(), type_name.end(), type_name.begin(), [](char c){ return std::tolower(c); });
    if(type_name == "directional") l.type = LightType::DIRECTIONAL;
    else if(type_name == "spot") l.type = LightType::SPOT;
    else l.type = LightType::POINT;
    l.color = j.value<glm::vec3>("color", {1,1,1});
    l.direction = j.value<glm::vec3>("direction", {0, -1, 0});
    l.position = j.value<glm::vec3>("position", {0,0,0});
    l.enabled = j.value("enabled", true);
    if(auto it = j.find("attenuation"); it != j.end()){
        auto& a = it.value();
        l.attenuation.constant = a.value("constant", 0.0f);
        l.attenuation.linear = a.value("linear", 0.0f);
        l.attenuation.quadratic = a.value("quadratic", 1.0f);
    } else {
        l.attenuation = {0.0f, 0.0f, 1.0f};
    }
    if(auto it = j.find("spot_angle"); it != j.end()){
        auto& a = it.value();
        l.spot_angle.inner = a.value("inner", glm::quarter_pi<float>());
        l.spot_angle.outer = a.value("outer", glm::half_pi<float>());
    } else {
        l.spot_angle = {glm::quarter_pi<float>(), glm::half_pi<float>()};
    }
}

struct SkyLight {
    bool enabled;
    glm::vec3 top_color, middle_color, bottom_color;
};

void from_json(const nlohmann::json& j, SkyLight& l){
    l.top_color = j.value<glm::vec3>("top_color", {0,0,0});
    l.middle_color = j.value<glm::vec3>("middle_color", {0.5,0.5,0.5});
    l.bottom_color = j.value<glm::vec3>("bottom_color", {1,1,1});
    l.enabled = j.value("enabled", true);
}

class TexturedMaterialApplication : public our::Application {

    our::ShaderProgram program, sky_program;

    std::unordered_map<std::string, std::unique_ptr<our::Mesh>> meshes;
    std::unordered_map<std::string, GLuint> textures;
    GLuint sampler;

    std::shared_ptr<Transform> root;

    our::Camera camera;
    our::FlyCameraController camera_controller;

    std::vector<Light> lights;
    SkyLight sky_light;
    float sky_box_exposure = 2.0f;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Textured Material", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex29_light/light_transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex32_textured_material/light_array.frag", GL_FRAGMENT_SHADER);
        program.link();
        sky_program.create();
        sky_program.attach("assets/shaders/ex32_textured_material/sky_transform.vert", GL_VERTEX_SHADER);
        sky_program.attach("assets/shaders/ex32_textured_material/sky.frag", GL_FRAGMENT_SHADER);
        sky_program.link();

        meshes["suzanne"] = std::make_unique<our::Mesh>();
        our::mesh_utils::loadOBJ(*(meshes["suzanne"]), "assets/models/Suzanne/Suzanne.obj");
        meshes["house"] = std::make_unique<our::Mesh>();
        our::mesh_utils::loadOBJ(*(meshes["house"]), "assets/models/House/House.obj");
        meshes["plane"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Plane(*(meshes["plane"]), {1, 1}, false, {0, 0, 0}, {1, 1}, {0, 0}, {100, 100});
        meshes["sphere"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Sphere(*(meshes["sphere"]), {32, 16}, false);
        meshes["cube"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Cuboid(*(meshes["cube"]));

        GLuint texture;

        glGenTextures(1, &texture);
        our::texture_utils::singleColor(texture, {255, 255, 255, 255});
        textures["white"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::singleColor(texture, {0, 0, 0, 255});
        textures["black"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::checkerBoard(texture, {256,256}, {128,128}, {255, 255, 255, 255}, {16, 16, 16, 255});
        textures["checkerboard_albedo"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::checkerBoard(texture, {256,256}, {128,128}, {0, 0, 0, 255}, {255, 255, 255, 255});
        textures["checkerboard_specular"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::checkerBoard(texture, {256,256}, {128,128}, {255, 255, 255, 255}, {64, 64, 64, 255});
        textures["checkerboard_roughness"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/materials/asphalt/albedo.jpg");
        textures["asphalt_albedo"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/materials/asphalt/specular.jpg");
        textures["asphalt_specular"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImageGrayscale(texture, "assets/images/common/materials/asphalt/roughness.jpg");
        textures["asphalt_roughness"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/materials/asphalt/emissive.jpg");
        textures["asphalt_emissive"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/materials/metal/albedo.jpg");
        textures["metal_albedo"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/materials/metal/specular.jpg");
        textures["metal_specular"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImageGrayscale(texture, "assets/images/common/materials/metal/roughness.jpg");
        textures["metal_roughness"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/materials/wood/albedo.jpg");
        textures["wood_albedo"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/materials/wood/specular.jpg");
        textures["wood_specular"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImageGrayscale(texture, "assets/images/common/materials/wood/roughness.jpg");
        textures["wood_roughness"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImageGrayscale(texture, "assets/images/common/materials/suzanne/ambient_occlusion.jpg");
        textures["suzanne_ambient_occlusion"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/models/House/House.jpeg");
        textures["house"] = texture;

        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/moon.jpg");
        textures["moon"] = texture;

        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
        GLfloat max_anisotropy_upper_bound = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy_upper_bound);
        glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotropy_upper_bound);
        for(GLuint unit = 0; unit < 5; ++unit) glBindSampler(unit, sampler);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        camera_controller.initialize(this, &camera);
        camera_controller.setFieldOfViewSensitivity(0.05f );

        std::ifstream file_in("assets/data/ex32_textured_material/scene.json");
        nlohmann::json json;
        file_in >> json;
        file_in.close();
        root = loadNode(json);

        file_in.open("assets/data/ex32_textured_material/lights.json");
        file_in >> json;
        file_in.close();
        sky_light = json.value("sky", SkyLight());
        lights = json.value("lights", lights);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glClearColor(0.0,0.0,0.0, 1);
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

    GLuint getTexture(const std::string& name){
        if(auto it = textures.find(name); it != textures.end())
            return it->second;
        else
            return 0;
    }

    void drawNode(const std::shared_ptr<Transform>& node, const glm::mat4& parent_transform_matrix, our::ShaderProgram& program){
        glm::mat4 transform_matrix = parent_transform_matrix * node->to_mat4();
        if(node->mesh.has_value()){
            if(auto mesh_it = meshes.find(node->mesh.value()); mesh_it != meshes.end()) {
                program.set("object_to_world", transform_matrix);
                program.set("object_to_world_inv_transpose", glm::inverse(transform_matrix), true);
                program.set("material.albedo_tint", node->material.albedo_tint);
                program.set("material.specular_tint", node->material.specular_tint);
                program.set("material.roughness_range", node->material.roughness_range);
                program.set("material.emissive_tint", node->material.emissive_tint);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, getTexture(node->material.albedo_map));
                program.set("material.albedo_map", 0);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, getTexture(node->material.specular_map));
                program.set("material.specular_map", 1);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, getTexture(node->material.ambient_occlusion_map));
                program.set("material.ambient_occlusion_map", 2);
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, getTexture(node->material.roughness_map));
                program.set("material.roughness_map", 3);
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, getTexture(node->material.emissive_map));
                program.set("material.emissive_map", 4);
                mesh_it->second->draw();
            }
        }
        for(auto& [name, child]: node->children){
            drawNode(child, transform_matrix, program);
        }
    }

    void onDraw(double deltaTime) override {
        camera_controller.update(deltaTime);


        glUseProgram(program);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program.set("camera_position", camera.getEyePosition());
        program.set("view_projection", camera.getVPMatrix());
        program.set("sky_light.top_color", sky_light.enabled ? sky_light.top_color : glm::vec3(0.0f));
        program.set("sky_light.middle_color", sky_light.enabled ? sky_light.middle_color : glm::vec3(0.0f));
        program.set("sky_light.bottom_color", sky_light.enabled ? sky_light.bottom_color : glm::vec3(0.0f));

        int light_index = 0;
        const int MAX_LIGHT_COUNT = 16;
        for(const auto& light : lights) {
            if(!light.enabled) continue;
            std::string prefix = "lights[" + std::to_string(light_index) + "].";

            program.set(prefix + "type", static_cast<int>(light.type));
            program.set(prefix + "color", light.color);

            switch (light.type) {
                case LightType::DIRECTIONAL:
                    program.set(prefix + "direction", glm::normalize(light.direction));
                    break;
                case LightType::POINT:
                    program.set(prefix + "position", light.position);
                    program.set(prefix + "attenuation_constant", light.attenuation.constant);
                    program.set(prefix + "attenuation_linear", light.attenuation.linear);
                    program.set(prefix + "attenuation_quadratic", light.attenuation.quadratic);
                    break;
                case LightType::SPOT:
                    program.set(prefix + "position", light.position);
                    program.set(prefix + "direction", glm::normalize(light.direction));
                    program.set(prefix + "attenuation_constant", light.attenuation.constant);
                    program.set(prefix + "attenuation_linear", light.attenuation.linear);
                    program.set(prefix + "attenuation_quadratic", light.attenuation.quadratic);
                    program.set(prefix + "inner_angle", light.spot_angle.inner);
                    program.set(prefix + "outer_angle", light.spot_angle.outer);
                    break;
            }
            light_index++;
            if(light_index >= MAX_LIGHT_COUNT) break;
        }
        program.set("light_count", light_index);

        drawNode(root, glm::mat4(1.0f), program);

        glUseProgram(sky_program);

        sky_program.set("view_projection", camera.getVPMatrix());
        sky_program.set("camera_position", camera.getEyePosition());
        sky_program.set("sky_light.top_color", sky_light.enabled ? sky_light.top_color : glm::vec3(0.0f));
        sky_program.set("sky_light.middle_color", sky_light.enabled ? sky_light.middle_color : glm::vec3(0.0f));
        sky_program.set("sky_light.bottom_color", sky_light.enabled ? sky_light.bottom_color : glm::vec3(0.0f));
        sky_program.set("exposure", sky_box_exposure);

        glCullFace(GL_FRONT);
        meshes["cube"]->draw();
        glCullFace(GL_BACK);
    }

    void onDestroy() override {
        program.destroy();
        sky_program.destroy();
        for(auto& [name, mesh]: meshes){
            mesh->destroy();
        }
        meshes.clear();
    }

    void displayNodeGui(const std::shared_ptr<Transform>& node, const std::string& node_name){
        if(ImGui::TreeNode(node_name.c_str())){
            if(node->mesh.has_value()) {
                our::PairIteratorCombo("Mesh", node->mesh.value(), meshes.begin(), meshes.end());
                our::PairIteratorCombo("Albedo Map", node->material.albedo_map, textures.begin(), textures.end());
                ImGui::ColorEdit3("Albedo Tint", glm::value_ptr(node->material.albedo_tint), ImGuiColorEditFlags_HDR);
                our::PairIteratorCombo("Specular Map", node->material.specular_map, textures.begin(), textures.end());
                ImGui::ColorEdit3("Specular Tint", glm::value_ptr(node->material.specular_tint), ImGuiColorEditFlags_HDR);
                our::PairIteratorCombo("Ambient Occlusion Map", node->material.ambient_occlusion_map, textures.begin(), textures.end());
                our::PairIteratorCombo("Emissive Map", node->material.emissive_map, textures.begin(), textures.end());
                ImGui::ColorEdit3("Emissive Tint", glm::value_ptr(node->material.emissive_tint), ImGuiColorEditFlags_HDR);
                our::PairIteratorCombo("Roughness Map", node->material.roughness_map, textures.begin(), textures.end());
                ImGui::DragFloatRange2("Roughness Range", &(node->material.roughness_range.x), &(node->material.roughness_range.y), 0.01f, 0.0f, 1.0f);
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

        ImGui::Begin("Lights");

        ImGui::Checkbox("Enable Sky Light", &sky_light.enabled);
        ImGui::ColorEdit3("Sky Top Color", glm::value_ptr(sky_light.top_color), ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit3("Sky Middle Color", glm::value_ptr(sky_light.middle_color), ImGuiColorEditFlags_HDR);
        ImGui::ColorEdit3("Sky Bottom Color", glm::value_ptr(sky_light.bottom_color), ImGuiColorEditFlags_HDR);
        ImGui::DragFloat("Sky Box Exposure (Background Only)", &sky_box_exposure, 0.1f);

        ImGui::Separator();

        our::ReorderableList(lights.begin(), lights.end(),
                             [](size_t index, Light& light){
                                    ImGui::Checkbox("Enabled", &light.enabled);
                                     if(ImGui::BeginCombo("Type", light_type_names.at(light.type))){
                                         for(auto& [type, name] : light_type_names){
                                             bool selected = light.type == type;
                                             if(ImGui::Selectable(name, selected))
                                                 light.type = type;
                                             if(selected) ImGui::SetItemDefaultFocus();
                                         }
                                         ImGui::EndCombo();
                                     }

                                     ImGui::ColorEdit3("Color", glm::value_ptr(light.color), ImGuiColorEditFlags_HDR);

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
        },
        [this](size_t index){
                lights.insert(lights.begin() + index, Light());
            },
        [this](size_t index){
            lights.erase(lights.begin() + index);
        });

        ImGui::End();

        ImGui::Begin("Scene");

        displayNodeGui(root, "root");

        ImGui::End();

    }

};

int main(int argc, char** argv) {
    return TexturedMaterialApplication().run();
}
