#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <mesh/mesh-utils.hpp>
#include <mesh/common-vertex-types.hpp>
#include <mesh/common-vertex-attributes.hpp>
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

class DepthTestingAndFaceCullingApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh triangle, model;

    std::vector<Transform> objects;
    Transform triangle_transform;

    our::Camera camera;
    our::FlyCameraController camera_controller;

    bool enable_depth_test = false;
    GLenum depth_function = GL_LEQUAL;
    bool enable_face_culling = false;
    GLenum culled_face = GL_BACK;
    GLenum front_face_winding = GL_CCW;
    bool draw_triangle = true;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Face Culling", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex11_transformation/transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex11_transformation/tint.frag", GL_FRAGMENT_SHADER);
        program.link();

        triangle.create({our::setup_buffer_accessors<our::ColoredVertex>});
        triangle.setVertexData<our::ColoredVertex>(0, {
                {{-0.5, -0.5, 0},{255,   0,   0, 255}},
                {{ 0.5, -0.5, 0},{  0, 255,   0, 255}},
                {{ 0.0,  0.5, 0},{  0,   0, 255, 255}}
        },GL_STATIC_DRAW);
        triangle.setElementData<GLuint>({
                                            0, 1, 2
                                    },GL_STATIC_DRAW);

        our::mesh_utils::Cuboid(model, true);

        objects.push_back({ {0,-1,0}, {0,0,0}, {11,2,11} });
        objects.push_back({ {-4,1,-4}, {0,0,0}, {2,2,2} });
        objects.push_back({ {4,1,-4}, {0,0,0}, {2,2,2} });
        objects.push_back({ {-4,1,4}, {0,0,0}, {2,2,2} });
        objects.push_back({ {4,1,4}, {0,0,0}, {2,2,2} });

        triangle_transform = { {0,1,0}, {0,0,0}, {2,2,2} };

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

        if(enable_face_culling) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        glCullFace(culled_face);
        glFrontFace(front_face_winding);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        program.set("tint", glm::vec4(1,1,1,1));

        for(const auto& object : objects) {
            program.set("transform", camera.getVPMatrix() * object.to_mat4());
            model.draw();
        }

        if(draw_triangle){
            program.set("transform", camera.getVPMatrix() * triangle_transform.to_mat4());
            triangle.draw();
        }
    }

    void onDestroy() override {
        program.destroy();
        model.destroy();
        triangle.destroy();
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

        ImGui::Text("Face Culling");

        ImGui::Checkbox("Enable Face Culling", &enable_face_culling);
        our::OptionMapCombo("Face To Cull", culled_face, our::gl_enum_options::facets);
        our::OptionMapCombo("Front Face", front_face_winding, our::gl_enum_options::face_windings);

        ImGui::Separator();

        ImGui::Text("Triangle Vertex Order: Red -> Green -> Blue");

        ImGui::DragFloat3("Translation", glm::value_ptr(triangle_transform.translation), 1.0f);
        ImGui::DragFloat3("Rotation", glm::value_ptr(triangle_transform.rotation), 0.1f);
        ImGui::DragFloat3("Scale", glm::value_ptr(triangle_transform.scale), 0.1f);
        
        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return DepthTestingAndFaceCullingApplication().run();
}
