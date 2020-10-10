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

class CameraStackApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh model;

    std::vector<Transform> objects;
    Transform weapon, button;

    our::Camera main_camera, weapon_camera;
    glm::mat4 ui_camera_matrix;
    our::FlyCameraController main_camera_controller;

    int clicks = 0;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Camera Stacking", {1280, 720}, false };
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

        weapon = {
                {1, -1, -1},
                {glm::pi<float>()/4, glm::pi<float>()/4, 0},
                {0.1f, 0.1f, 2}
        };

        button = {
                {100, 100, -100},
                {0, 0, 0},
                {150, 150, 1}
        };

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        main_camera.setEyePosition({10, 10, 10});
        main_camera.setTarget({0, 0, 0});
        main_camera.setUp({0, 1, 0});
        main_camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        main_camera_controller.initialize(this, &main_camera);

        weapon_camera.setEyePosition({0, 0, 0});
        weapon_camera.setTarget({0, 0, -1});
        weapon_camera.setUp({0, 1, 0});
        weapon_camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        ui_camera_matrix =
                glm::ortho(0.0f, (float)width, 0.0f, (float)height, 0.0f, 1000.0f) *
                glm::lookAt(glm::vec3(0,0,0), glm::vec3(0,0,-1), glm::vec3(0,1,0));


        glClearColor(0, 0, 0, 0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glClearColor(0, 0, 0, 1);
    }

    void onDraw(double deltaTime) override {
        main_camera_controller.update(deltaTime);
        weapon.rotation.z += deltaTime;

        glm::vec4 button_tint = {0.1f, 0.1f, 0.1f, 1.0f};

        auto mouse_position = mouse.getMousePosition();
        mouse_position.y = getFrameBufferSize().y - mouse_position.y;
        if( abs(mouse_position.x - button.translation.x) < button.scale.x * 0.5f &&
            abs(mouse_position.y - button.translation.y) < button.scale.y * 0.5f) {
            button_tint = {0.3f, 0.3f, 0.3f, 1.0f};
            if(mouse.isPressed(0))
                button_tint = {0.6f, 0.6f, 0.6f, 1.0f};
            if(mouse.justPressed(0))
                clicks++;
        }

        glUseProgram(program);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program.set("tint", glm::vec4(1, 1, 1, 1));

        for (const auto &object : objects) {
            program.set("transform", main_camera.getVPMatrix() * object.to_mat4());
            model.draw();
        }

        glClear(GL_DEPTH_BUFFER_BIT);

        program.set("tint", glm::vec4(0.2, 0.2, 0.2, 1));
        program.set("transform", weapon_camera.getVPMatrix() * weapon.to_mat4());
        model.draw();

        glClear(GL_DEPTH_BUFFER_BIT);

        program.set("tint", button_tint);
        program.set("transform",  ui_camera_matrix * button.to_mat4());
        model.draw();

    }

    void onDestroy() override {
        program.destroy();
        model.destroy();
    }

    void onImmediateGui(ImGuiIO &io) override {

        ImGui::Begin("Watch");

        ImGui::Text("Clicks: %i", clicks);

        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return CameraStackApplication().run();
}
