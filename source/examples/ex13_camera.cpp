#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <mesh/common-vertex-types.hpp>
#include <mesh/common-vertex-attributes.hpp>

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

class CameraApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh quad;

    std::vector<Transform> objects;
    Transform camera;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Camera (Simple)", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex11_transformation/transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex11_transformation/tint.frag", GL_FRAGMENT_SHADER);
        program.link();

        quad.create({our::setup_buffer_accessors<our::ColoredVertex>});
        quad.setVertexData<our::ColoredVertex>(0, {
                {{-0.5, -0.5, 0},{255,   0,   0, 255}},
                {{ 0.5, -0.5, 0},{  0, 255,   0, 255}},
                {{ 0.5,  0.5, 0},{  0,   0, 255, 255}},
                {{-0.5,  0.5, 0},{255, 255,   0, 255}}
        },GL_STATIC_DRAW);
        quad.setElementData<GLuint>({
                                            0, 1, 2,
                                            2, 3, 0
                                    },GL_STATIC_DRAW);

        objects.push_back({ {0,-100,0}, {0,0,0}, {500,20,1} });
        objects.push_back({ {-200,100,0}, {0,0,0}, {30,30,1} });
        objects.push_back({ {0,100,0}, {0,0,glm::pi<float>()/4}, {30,30,1} });
        objects.push_back({ {200,100,0}, {0,0,0}, {30,30,1} });

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        camera = Transform({0,0,0},{0, 0, 0},{width, height, 1});

        glClearColor(0, 0, 0, 0);
    }

    void onDraw(double deltaTime) override {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);

        program.set("tint", glm::vec4(1,1,1,1));

        glm::mat4 camera_matrix = glm::inverse(camera.to_mat4());

        for(const auto& object : objects) {
            program.set("transform", camera_matrix * object.to_mat4());
            quad.draw();
        }
    }

    void onDestroy() override {
        program.destroy();
        quad.destroy();
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

        ImGui::Begin("Camera");
        ImGui::DragFloat3("Translation", glm::value_ptr(camera.translation), 1.0f);
        ImGui::DragFloat3("Rotation", glm::value_ptr(camera.rotation), 0.1f);
        ImGui::DragFloat3("Scale", glm::value_ptr(camera.scale), 0.1f);
        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return CameraApplication().run();
}
