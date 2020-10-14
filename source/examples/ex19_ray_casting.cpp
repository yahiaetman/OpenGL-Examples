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
#include <glm/gtx/intersect.hpp>

#include <random>

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

class RayCastingApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh model, rays;

    std::vector<Transform> objects;

    our::Camera camera;
    our::FlyCameraController controller;

    std::vector<our::Vertex> model_vertices;
    std::vector<our::ColoredVertex> ray_vertices;
    std::vector<GLuint> model_elements;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Ray Casting", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex11_transformation/transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex11_transformation/tint.frag", GL_FRAGMENT_SHADER);
        program.link();

        our::mesh_utils::Cuboid(model, true);
        model.getElementData(model_elements);
        model.getVertexData(0, model_vertices);

        rays.create({our::setup_buffer_accessors<our::ColoredVertex>}, false);
        rays.setPrimitiveMode(GL_LINES);

        std::mt19937_64 random_generator(1234);
        std::uniform_real_distribution y_generator(1.0f, 5.0f);
        std::uniform_real_distribution angle_generator(0.0f, glm::pi<float>() * 2);
        std::uniform_real_distribution scale_generator(0.5f, 4.0f);

        for(int x = -24; x <= 24; x += 8) {
            for (int z = -24; z <= 24; z += 8) {
                float y = y_generator(random_generator);
                glm::vec3 rotation = { angle_generator(random_generator), angle_generator(random_generator), angle_generator(random_generator) };
                glm::vec3 scale = { scale_generator(random_generator), scale_generator(random_generator), scale_generator(random_generator) };
                objects.push_back({{x, y, z},
                                   rotation, scale});
            }
        }

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        controller.initialize(this, &camera);

        glClearColor(0, 0, 0, 0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glClearColor(0, 0, 0, 1);
    }

    void onDraw(double deltaTime) override {
        controller.update(deltaTime);

        glm::vec2 mouse_window_space = mouse.getMousePosition();
        glm::ivec2 window_size = getWindowSize();
        glm::vec4 mouse_ndc_near = {
                2 * (mouse_window_space.x / window_size.x) - 1,
                1 - 2 * (mouse_window_space.y / window_size.y),
                -1,
                1
        };
        glm::vec4 mouse_ndc_far = { mouse_ndc_near.x, mouse_ndc_near.y, 1, 1 };
        glm::mat4 VP_inverse = glm::inverse(camera.getVPMatrix());
        glm::vec4 mouse_world_near = VP_inverse * mouse_ndc_near, mouse_world_far = VP_inverse * mouse_ndc_far;
        mouse_world_near /= mouse_world_near.w;
        mouse_world_far /= mouse_world_far.w;

        glm::vec3 ray_origin = mouse_world_near, ray_direction = glm::normalize(mouse_world_far - mouse_world_near);

        float nearest_hit_distance = std::numeric_limits<float>::infinity();
        glm::vec3 nearest_hit_point = mouse_world_far;
        size_t hit_transform_index = -1;
        for(size_t index = 0; index < objects.size(); ++index){
            auto& transform = objects[index];
            glm::mat4 world_matrix = transform.to_mat4();
            size_t element_count = (model_elements.size() / 3) * 3;
            for(size_t element = 0; element < element_count;){
                glm::vec3 v0 = world_matrix * glm::vec4(model_vertices[model_elements[element++]].position, 1.0f);
                glm::vec3 v1 = world_matrix * glm::vec4(model_vertices[model_elements[element++]].position, 1.0f);
                glm::vec3 v2 = world_matrix * glm::vec4(model_vertices[model_elements[element++]].position, 1.0f);
                glm::vec2 barycentric_coords; float triangle_hit_distance;
                if(glm::intersectRayTriangle(ray_origin, ray_direction, v0, v1, v2, barycentric_coords, triangle_hit_distance)){
                    if(triangle_hit_distance > 0 && triangle_hit_distance < nearest_hit_distance){
                        nearest_hit_distance = triangle_hit_distance;
                        hit_transform_index = index;
                        nearest_hit_point =
                                v1 * barycentric_coords.x +
                                v2 * barycentric_coords.y +
                                v0 * (1 - barycentric_coords.x - barycentric_coords.y);
                    }
                }
            }
        }

        if(mouse.justPressed(1)){
            ray_vertices.push_back({ ray_origin, {255, 196, 128, 255}});
            ray_vertices.push_back({ nearest_hit_point, {196, 128, 255, 255}});
            rays.setVertexData(0, ray_vertices, GL_DYNAMIC_DRAW);
            rays.setVertexCount(ray_vertices.size());
        }

        glUseProgram(program);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        for (size_t index = 0; index < objects.size(); ++index) {
            const auto& object = objects[index];
            glm::vec4 tint = (index == hit_transform_index) ? glm::vec4(1,1,1,1) : glm::vec4(0.2,0.2,0.2,1);
            program.set("tint", tint);
            program.set("transform", camera.getVPMatrix() * object.to_mat4());
            model.draw();
        }

        program.set("tint", glm::vec4(1, 1, 1, 1));
        program.set("transform", camera.getVPMatrix());
        rays.draw();
    }

    void onDestroy() override {
        program.destroy();
        model.destroy();
    }

};

int main(int argc, char** argv) {
    return RayCastingApplication().run();
}
