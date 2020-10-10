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

struct RenderArea {
    glm::ivec2 viewport_origin, viewport_size;
    glm::ivec2 scissors_origin, scissors_size;
    bool enable_scissors;
    glm::vec4 clear_color;
    bool clear;
    our::Camera camera;
    our::FlyCameraController controller;
};

class ViewportsApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh model;

    std::vector<Transform> objects;
    std::vector<RenderArea> areas;
    size_t selected_camera;

    our::Camera default_camera;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Viewports and Scissors", {1280, 720}, false };
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

        default_camera.setEyePosition({10, 10, 10});
        default_camera.setTarget({0, 0, 0});
        default_camera.setUp({0, 1, 0});
        default_camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        glm::ivec2 half_size(width/2, height/2);

        areas.push_back({
                glm::ivec2(0,0), half_size,
                glm::ivec2(0, 0), half_size,
                true,
                glm::vec4(0.1f, 0.2f, 0.3f, 1.0f),
                true,
                default_camera, {}
            });
        areas.push_back({
                half_size, half_size,
                half_size, half_size,
                true,
                glm::vec4(0.3f, 0.2f, 0.1f, 1.0f),
                true,
                default_camera, {}
        });
        for(auto& area: areas){
            area.controller.initialize(this, &area.camera);
        }

        glClearColor(0, 0, 0, 0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    void onDraw(double deltaTime) override {
        if(areas.size() > 0)
            areas[selected_camera].controller.update(deltaTime);

        glUseProgram(program);
        program.set("tint", glm::vec4(1, 1, 1, 1));

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(auto& area: areas) {
            glViewport(area.viewport_origin.x, area.viewport_origin.y, area.viewport_size.x, area.viewport_size.y);

            if(area.enable_scissors) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
            glScissor(area.scissors_origin.x, area.scissors_origin.y, area.scissors_size.x, area.scissors_size.y);

            if(area.clear) {
                glClearColor(area.clear_color.r, area.clear_color.g, area.clear_color.b, area.clear_color.a);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }

            for (const auto &object : objects) {
                program.set("transform", area.camera.getVPMatrix() * object.to_mat4());
                model.draw();
            }
        }

        //NOTE: Remember to reset the viewport and scissors such that ImGUI can draw
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glDisable(GL_SCISSOR_TEST);
        glScissor(0, 0, width, height);
    }

    void onDestroy() override {
        program.destroy();
        model.destroy();
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

        ImGui::Begin("Areas");

        our::ReorderableList(areas.begin(), areas.end(),
             [this](size_t index, RenderArea& area){
                if(this->selected_camera != index && ImGui::Button("Take Control over Camera"))
                    this->selected_camera = index;
                ImGui::DragInt2("Viewport Origin", glm::value_ptr(area.viewport_origin));
                ImGui::DragInt2("Viewport Size", glm::value_ptr(area.viewport_size));
                ImGui::Checkbox("Enabled Scissor Test", &area.enable_scissors);
                ImGui::DragInt2("Scissors Origin", glm::value_ptr(area.scissors_origin));
                ImGui::DragInt2("Scissors Size", glm::value_ptr(area.scissors_size));
                ImGui::Checkbox("Clear", &area.clear);
                ImGui::ColorEdit4("Clear Color", glm::value_ptr(area.clear_color));
        }, [this](size_t index){
                    areas.insert(areas.begin() + index, {
                            {0,0},{100,100},
                            {0,0},{100,100},
                            true, {1,1,1,1}, true, this->default_camera, {}
                    });
                    areas[index].controller.initialize(this, &(areas[index].camera));
        }, [this](size_t index){
                    areas.erase(areas.begin() + index);
                    if(areas.size() <= this->selected_camera) this->selected_camera = 0;
        });

        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return ViewportsApplication().run();
}
