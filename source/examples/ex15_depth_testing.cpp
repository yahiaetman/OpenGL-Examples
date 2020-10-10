#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <mesh/mesh-utils.hpp>
#include <camera/camera.hpp>
#include <camera/controllers/fly_camera_controller.hpp>

#include <glm/gtx/euler_angles.hpp>

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

class DepthTestingApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh model;

    std::vector<Transform> objects;

    our::Camera camera;
    our::FlyCameraController camera_controller;

    bool enable_depth_test = false;
    GLenum depth_function = GL_LEQUAL;
    float clear_depth = 1.0f;
    bool depth_mask = true;
    glm::bvec4 color_mask = {true, true, true, true};

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Depth Testing", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex11_transformation/transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex11_transformation/tint.frag", GL_FRAGMENT_SHADER);
        program.link();

        our::mesh_utils::Cuboid(model, true);

        objects.push_back({ {0,-1,0}, {0,0,0}, {7,2,7} });
        objects.push_back({ {-2,1,-2}, {0,0,0}, {2,2,2} });
        objects.push_back({ {2,1,-2}, {0,0,0}, {2,2,2} });
        objects.push_back({ {-2,1,2}, {0,0,0}, {2,2,2} });
        objects.push_back({ {2,1,2}, {0,0,0}, {2,2,2} });

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        camera_controller.initialize(this, &camera);

        glClearColor(0, 0, 0, 0);
    }

    void onDraw(double deltaTime) override {
        camera_controller.update(deltaTime);

        if(enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        glDepthFunc(depth_function);

        glClearDepth(clear_depth);

        glDepthMask(depth_mask);
        glColorMask(color_mask.r, color_mask.g, color_mask.b, color_mask.a);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        program.set("tint", glm::vec4(1,1,1,1));

        for(const auto& object : objects) {
            program.set("transform", camera.getVPMatrix() * object.to_mat4());
            model.draw();
        }

        //NOTE: Remember to reset the color mask such that ImGUI can draw
        glColorMask(true, true, true, true);
    }

    void onDestroy() override {
        program.destroy();
        model.destroy();
        camera_controller.release();
    }

    void onImmediateGui(ImGuiIO &io) override {

        ImGui::Begin("Objects");

        our::ReorderableList(objects.begin(), objects.end(),
                             [](size_t index, Transform& transform){
            ImGui::DragFloat3("Translation", glm::value_ptr(transform.translation), 1.0f);
            ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), 0.1f);
            ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.1f);
        }, [this](size_t index){
            objects.insert(objects.begin() + index, Transform());
        }, [this](size_t index){
            objects.erase(objects.begin() + index);
        });

        ImGui::End();

        ImGui::Begin("Controls");

        ImGui::Text("Depth Testing");

        ImGui::Checkbox("Enable Depth Testing", &enable_depth_test);
        our::OptionMapCombo("Comparison Function", depth_function, our::gl_enum_options::comparison_functions);

        ImGui::Separator();

        ImGui::Text("Extras");

        ImGui::DragFloat("Clear Depth Value", &clear_depth, 0.01f, 0.0f, 1.0f);
        ImGui::Checkbox("Depth Mask", &depth_mask);
        ImGui::Checkbox("Color Mask (Red)", &color_mask.r);
        ImGui::Checkbox("Color Mask (Green)", &color_mask.g);
        ImGui::Checkbox("Color Mask (Blue)", &color_mask.b);
        ImGui::Checkbox("Color Mask (Alpha)", &color_mask.a);

        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return DepthTestingApplication().run();
}
