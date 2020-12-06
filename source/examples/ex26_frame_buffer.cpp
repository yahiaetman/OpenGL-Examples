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

struct Transform {
    glm::vec4 tint;
    glm::vec3 translation, rotation, scale;
    std::optional<std::string> mesh;
    std::string texture;
    std::unordered_map<std::string, std::shared_ptr<Transform>> children;


    explicit Transform(
            const glm::vec4& tint = {1,1,1,1},
            const glm::vec3& translation = {0,0,0},
            const glm::vec3& rotation = {0,0,0},
            const glm::vec3& scale = {1,1,1},
            std::optional<std::string>  mesh = std::nullopt,
            std::string  texture = ""
    ): tint(tint), translation(translation), rotation(rotation), scale(scale), mesh(std::move(mesh)), texture(std::move(texture)) {}

    [[nodiscard]] glm::mat4 to_mat4() const {
        return glm::translate(glm::mat4(1.0f), translation) *
               glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

// This example demonstrates how to use Frame Buffer Objects to render to an off-screen texture.
class FrameBufferApplication : public our::Application {

    our::ShaderProgram program;

    std::unordered_map<std::string, std::unique_ptr<our::Mesh>> meshes;

    std::unordered_map<std::string, GLuint> textures;

    GLuint sampler = 0;

    std::shared_ptr<Transform> root, internal_root;

    our::Camera camera, internal_camera;
    our::FlyCameraController camera_controller, internal_camera_controller;

    bool control_internal_camera = false;

    // Since frame buffers are OpenGL objects, we identify them using a GLuint.
    GLuint frame_buffer = 0;

    // This will be the size of the texture that we will render to.
    const glm::ivec2 rt_size = {512, 512};

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Frame Buffer", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        // Nothing unusual about our shader. We don't need any specific shader code to support frame buffers.
        program.attach("assets/shaders/ex22_texture_sampling/transform.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex22_texture_sampling/texture.frag", GL_FRAGMENT_SHADER);
        program.link();

        GLuint texture;

        glGenTextures(1, &texture);
        our::texture_utils::checkerBoard(texture, {256, 256}, {128, 128}, {255, 255, 255, 255}, {16, 16, 16, 255});
        textures["checkerboard"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/models/House/House.jpeg");
        textures["house"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/moon.jpg");
        textures["moon"] = texture;

        // Here we calculate the number of mip levels needed to create full mip map starting from the full size (size of level 0).
        GLuint rt_levels = glm::floor(glm::log2(glm::max<float>(rt_size.x, rt_size.y))) + 1;
        // Here we will create a texture to hold the color that we will render to the framebuffer.
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // Since we don't need to send any data to the texture while creation, we don't need to use glTexImage2D.
        // Instead, we can use glTexStorage2D which allocates the storages but doesn't put any data in it.
        // The parameters are the target to allocate storage for, the number of mip levels to allocate, the internal format and the width & height of mip level 0.
        glTexStorage2D(GL_TEXTURE_2D, rt_levels, GL_RGBA8, rt_size.x, rt_size.y);
        textures["color_rt"] = texture;
        // We also need to create a depth texture to store the pixel depth. This is only needed if we plan to depth testing.
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        // Since it is a depth texture, we choose a format that is designed for depth textures. Here we use a 32-bit depth component.
        // We don't plan to render this texture on objects, so it would be useless to have a mip map for it.
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32, rt_size.x, rt_size.y);
        textures["depth_rt"] = texture;

        meshes["house"] = std::make_unique<our::Mesh>();
        our::mesh_utils::loadOBJ(*(meshes["house"]), "assets/models/House/House.obj");
        meshes["plane"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Plane(*(meshes["plane"]), {1, 1}, false, {0, 0, 0}, {1, 1}, {0, 0}, {100, 100});
        meshes["sphere"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Sphere(*(meshes["sphere"]), {32, 16}, false);
        meshes["cube"] = std::make_unique<our::Mesh>();
        our::mesh_utils::Cuboid(*(meshes["cube"]));

        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindSampler(0, sampler);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // This camera will be used for rendering the scene to window.
        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>() / 2, static_cast<float>(width) / height, 0.1f, 100.0f);

        camera_controller.initialize(this, &camera);

        // This camera will be used for rendering the scene to the frame buffer.
        internal_camera.setEyePosition({10, 10, 10});
        internal_camera.setTarget({0, 0, 0});
        internal_camera.setUp({0, 1, 0});
        internal_camera.setupPerspective(glm::pi<float>() / 2, static_cast<float>(rt_size.x) / rt_size.y, 0.1f, 100.0f);

        internal_camera_controller.initialize(this, &internal_camera);

        // We have two scenes, one will be drawn on the window and the other will be drawn to the frame buffer.
        root = loadSceneGraph("assets/data/ex26_frame_buffer/external.json");
        internal_root = loadSceneGraph("assets/data/ex23_sampler_objects/scene.json");

        // First, we generate 1 frame buffer.
        glGenFramebuffers(1, &frame_buffer);
        // Then we bind it as our draw frame buffer. We can bind frame buffers to one of 3 targets:
        //  -   GL_DRAW_FRAMEBUFFER which is the frame buffer that will receive the fragment color from subsequent draw calls.
        //  -   GL_READ_FRAMEBUFFER which is the frame buffer from we will read the pixel in any subsequent pixel read or transfer operations.
        //  -   GL_FRAMEBUFFER which is both of the above.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer);
        // Now we attach our textures to framebuffer. We attach the color texture as a color attachment and the depth texture as a depth attachment.
        // The last parameter specifies the mip level that will be attached.
        // Note that we can have more that more than one color attachment by attaching to "GL_COLOR_ATTACHMENT1, .. ".
        // This useful for what is called multiple render targets where we draw to multiple textures using a single draw call.
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures["color_rt"], 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures["depth_rt"], 0);

        // Finally, we check that our frame buffer is complete (A.K.A. ready for rendering).
        // For a full checklist for frame buffer completeness, check https://www.khronos.org/opengl/wiki/Framebuffer_Object#Framebuffer_Completeness
        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            std::cerr << "Frame buffer is incomplete" << std::endl;
        }

        // Now we return to the default framebuffer (name = 0) which is the back buffer of our window.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    std::shared_ptr<Transform> loadNode(const nlohmann::json& json){
        auto node = std::make_shared<Transform>(
                json.value<glm::vec4>("tint", {1,1,1,1}),
                json.value<glm::vec3>("translation", {0, 0, 0}),
                json.value<glm::vec3>("rotation", {0, 0, 0}),
                json.value<glm::vec3>("scale", {1, 1, 1})
        );
        if(json.contains("mesh")){
            node->mesh = json["mesh"].get<std::string>();
        }
        if(json.contains("texture")){
            node->texture = json["texture"].get<std::string>();
        }
        if(json.contains("children")){
            for(auto& [name, child]: json["children"].items()){
                node->children[name] = loadNode(child);
            }
        }
        return node;
    }

    std::shared_ptr<Transform> loadSceneGraph(const std::string& filename){
        std::ifstream file_in(filename);
        nlohmann::json json;
        file_in >> json;
        file_in.close();
        return loadNode(json);
    }

    void drawNode(const std::shared_ptr<Transform>& node, const glm::mat4& parent_transform_matrix){
        glm::mat4 transform_matrix = parent_transform_matrix * node->to_mat4();
        if(node->mesh.has_value()){
            if(auto mesh_it = meshes.find(node->mesh.value()); mesh_it != meshes.end()) {
                GLuint texture = 0;
                if(auto tex_it = textures.find(node->texture); tex_it != textures.end())
                    texture = tex_it->second;
                glBindTexture(GL_TEXTURE_2D, texture);
                program.set("tint", node->tint);
                program.set("transform", transform_matrix);
                mesh_it->second->draw();
            }
        }
        for(auto& [name, child]: node->children){
            drawNode(child, transform_matrix);
        }
    }

    void onDraw(double deltaTime) override {
        if(control_internal_camera)
            internal_camera_controller.update(deltaTime);
        else
            camera_controller.update(deltaTime);

        root->children["moon-axis"]->children["moon"]->rotation.y += deltaTime;
        internal_root->children["moon-axis"]->children["moon"]->rotation.y += deltaTime;

        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
        program.set("sampler", 0);

        // First of all, to draw to a frame buffer, we need to bind it for drawing.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer);
        // Since the viewport transformation is configured by default to match the window, the NDC space may not be stretched correctly to match our framebuffer.
        // So we need to configure our viewport to match the framebuffer size.
        glViewport(0, 0, rt_size.x, rt_size.y);

        // Now we are clearing our frame buffer and not the window back buffer.
        glClearColor(0.88,0.65,0.15, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Then we draw the internal scene. This will be rendered to our frame buffer not the window back buffer.
        drawNode(internal_root, internal_camera.getVPMatrix());

        // Now, let's return to the window back buffer.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // Then we get the window width and height and reconfigure the viewport to match the window size.
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // Don't forget that we updated our color render target so before using it, it is necessary to re-generate the mip maps such that trilinear filtering would work correctly.
        glBindTexture(GL_TEXTURE_2D, textures["color_rt"]);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Now we are clearing the window back buffer.
        glClearColor(0.05,0.1,0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // This will be drawn to the window back buffer.
        drawNode(root, camera.getVPMatrix());
    }

    void onDestroy() override {
        program.destroy();
        glDeleteSamplers(1, &sampler);
        glDeleteFramebuffers(1, &frame_buffer);
        for(auto& [name, texture]: textures){
            glDeleteTextures(1, &texture);
        }
        textures.clear();
        for(auto& [name, mesh]: meshes){
            mesh->destroy();
        }
        meshes.clear();
    }

    void onImmediateGui(ImGuiIO &io) override {
        ImGui::Begin("Controls");

        ImGui::Checkbox("Control Internal Camera", &control_internal_camera);

        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return FrameBufferApplication().run();
}
