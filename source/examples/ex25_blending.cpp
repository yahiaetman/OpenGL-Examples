#include <application.hpp>
#include <shader.hpp>
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
    bool transparent;
    std::optional<std::string> mesh;
    std::string texture;
    std::unordered_map<std::string, std::shared_ptr<Transform>> children;

    explicit Transform(
            const glm::vec4& tint = {1,1,1,1},
            const glm::vec3& translation = {0,0,0},
            const glm::vec3& rotation = {0,0,0},
            const glm::vec3& scale = {1,1,1},
            bool transparent = false,
            const std::optional<std::string>& mesh = std::nullopt,
            const std::string& texture = ""
    ): tint(tint), translation(translation), rotation(rotation), scale(scale), transparent(transparent),
        mesh(mesh), texture(texture) {}

    [[nodiscard]] glm::mat4 to_mat4() const {
        return glm::translate(glm::mat4(1.0f), translation) *
               glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

// Since we need to sort the objects in our scene, it may not be possible to draw the objects in a sorted manner while traversing the scene graph.
// So instead, we store a what we call a MeshRenderCommand during the traversal and after we finish the traversal, we sort then draw.
// The MeshRenderCommand should store all the data needed to both sort and draw the objects.
struct MeshRenderCommand {
    bool transparent;
    float depth;
    glm::vec4 tint;
    glm::mat4 transformation;
    std::weak_ptr<our::Mesh> mesh;
    GLuint texture;

    bool operator<(const MeshRenderCommand& other) const {
        // Let opaque objects be drawn before the transparent ones
        if(transparent != other.transparent) return transparent < other.transparent;
        // If both are transparent, sort from farthest to nearest
        else if(transparent) return depth > other.depth;
        // If both are opaque, sort from nearest to farthest
        else return depth < other.depth;
    }
};

// This example demonstrates how to do blending and some of the solutions to problems that arise while drawing transparent objects.
class BlendingApplication : public our::Application {

    our::ShaderProgram default_program, alpha_test_program;

    std::unordered_map<std::string, std::shared_ptr<our::Mesh>> meshes;

    std::unordered_map<std::string, GLuint> textures;

    GLuint sampler = 0;

    std::shared_ptr<Transform> root;
    std::vector<MeshRenderCommand> render_commands;

    our::Camera camera;
    our::FlyCameraController camera_controller;

    bool enable_depth_test = true;
    GLenum depth_function = GL_LEQUAL;
    bool enable_transparent_depth_write = true;
    bool enable_face_culling = true;
    GLenum culled_face = GL_BACK;
    GLenum front_face_winding = GL_CCW;

    // These variables will control the blending
    bool enable_blending = false;
    GLenum blend_equation = GL_FUNC_ADD;
    GLenum blend_source_factor = GL_SRC_ALPHA, blend_destination_factor = GL_ONE_MINUS_SRC_ALPHA;
    glm::vec4 blend_constant_color = {1.0f,1.0f,1.0f,1.0f};

    // These variables will control the alpha testing
    bool enable_alpha_test = false;
    float alpha_test_threshold = 0.5;

    // This variable will enable or disable the alpha to coverage transparency
    bool enable_alpha_to_coverage = false;

    bool sort_render_commands = false;

    void configureOpenGL() override {
        our::Application::configureOpenGL();
        // While in most examples we don't enable MSAA, we will need it here for Alpha to Coverage.
        glfwWindowHint(GLFW_SAMPLES, 4);
    }

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Sampler Objects", {1280, 720}, false };
    }

    void onInitialize() override {
        default_program.create();
        // We don't need anything special in our shaders to support blending.
        default_program.attach("assets/shaders/ex22_texture_sampling/transform.vert", GL_VERTEX_SHADER);
        default_program.attach("assets/shaders/ex22_texture_sampling/texture.frag", GL_FRAGMENT_SHADER);
        default_program.link();
        alpha_test_program.create();
        alpha_test_program.attach("assets/shaders/ex22_texture_sampling/transform.vert", GL_VERTEX_SHADER);
        // However, alpha testing is implemented in the fragment shader so we need to use a special program for alpha testing.
        alpha_test_program.attach("assets/shaders/ex25_blending/alpha_test.frag", GL_FRAGMENT_SHADER);
        alpha_test_program.link();


        GLuint texture;
        glGenTextures(1, &texture);
        our::texture_utils::singleColor(texture, {255,255,255,255});
        textures["white"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::checkerBoard(texture, {256,256}, {128,128}, {255, 255, 255, 255}, {16, 16, 16, 255});
        textures["checkerboard"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/color-grid.png");
        textures["color-grid"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/common/moon.jpg");
        textures["moon"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/ex25_blending/glass-panels.png");
        textures["glass-panels"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/ex25_blending/metal.png");
        textures["metal"] = texture;
        glGenTextures(1, &texture);
        our::texture_utils::loadImage(texture, "assets/images/ex25_blending/fog.png");
        textures["fog"] = texture;

        meshes["cube"] = std::make_shared<our::Mesh>();
        our::mesh_utils::Cuboid(*(meshes["cube"]));
        meshes["plane"] = std::make_shared<our::Mesh>();
        our::mesh_utils::Plane(*(meshes["plane"]), {1, 1}, false, {0, 0, 0}, {1, 1}, {0, 0}, {100, 100});
        meshes["sphere"] = std::make_shared<our::Mesh>();
        our::mesh_utils::Sphere(*(meshes["sphere"]), {32, 16}, false);

        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.setEyePosition({10, 10, 10});
        camera.setTarget({0, 0, 0});
        camera.setUp({0, 1, 0});
        camera.setupPerspective(glm::pi<float>()/2, static_cast<float>(width)/height, 0.1f, 100.0f);

        camera_controller.initialize(this, &camera);
        camera_controller.setFieldOfViewSensitivity(0.05f );

        std::ifstream file_in("assets/data/ex25_blending/scene.json");
        nlohmann::json json;
        file_in >> json;
        file_in.close();
        root = loadNode(json);

        glClearColor(0.88,0.65,0.15, 1);
    }

    std::shared_ptr<Transform> loadNode(const nlohmann::json& json){
        auto node = std::make_shared<Transform>(
                json.value<glm::vec4>("tint", {1,1,1,1}),
                json.value<glm::vec3>("translation", {0, 0, 0}),
                json.value<glm::vec3>("rotation", {0, 0, 0}),
                json.value<glm::vec3>("scale", {1, 1, 1}),
                json.value<bool>("transparent", false)
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

    // This function traverses the scene graph and adds a render command for each node that should be drawn.
    void buildRenderCommands(const std::shared_ptr<Transform>& node, const glm::mat4& parent_transform_matrix){
        glm::mat4 transform_matrix = parent_transform_matrix * node->to_mat4();
        if(node->mesh.has_value()){
            if(auto mesh_it = meshes.find(node->mesh.value()); mesh_it != meshes.end()) {
                GLuint texture = 0;
                if(auto tex_it = textures.find(node->texture); tex_it != textures.end())
                    texture = tex_it->second;
                // We sort using the depth of the origin of the object. This is an assumption we picked since any point will have its drawback.
                glm::vec4 transformed_origin = transform_matrix * glm::vec4(0, 0, 0, 1);
                float depth = transformed_origin.z / transformed_origin.w;
                render_commands.push_back({
                    node->transparent,
                    depth,
                    node->tint,
                    transform_matrix,
                    mesh_it->second,
                    texture
                });
            }
        }
        for(auto& [name, child]: node->children){
            buildRenderCommands(child, transform_matrix);
        }
    }

    void onDraw(double deltaTime) override {
        camera_controller.update(deltaTime);

        // If alpha testing is enabled, we use the program that supports alpha testing
        auto& program = enable_alpha_test ? alpha_test_program : default_program;

        if(enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        glDepthFunc(depth_function);

        if(enable_face_culling) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        glCullFace(culled_face);
        glFrontFace(front_face_winding);

        // The blending formula has the following form: (source_factor * source) operation (destination_factor * destination).
        // The operation is specified by the blend equation function.
        // The possible values are:
        //  -   GL_FUNC_ADD: the operation is "+".
        //  -   GL_FUNC_SUBTRACT: the operation is "-".
        //  -   GL_FUNC_REVERSE_SUBTRACT: the operation is "-" but the operands are reversed.
        //  -   GL_MIN: the operation picks the minimum value among the 2 operands.
        //  -   GL_MAX: the operation picks the maximum value among the 2 operands.
        glBlendEquation(blend_equation);
        // This function specifies the source of the factors for each operand.
        // the possible values are:
        //  -   GL_ZERO
        //  -   GL_ONE
        //  -   GL_SRC_COLOR
        //  -   GL_ONE_MINUS_SRC_COLOR
        //  -   GL_DST_COLOR
        //  -   GL_ONE_MINUS_DST_COLOR
        //  -   GL_SRC_ALPHA
        //  -   GL_ONE_MINUS_SRC_ALPHA
        //  -   GL_DST_ALPHA
        //  -   GL_ONE_MINUS_DST_ALPHA
        //  -   GL_CONSTANT_COLOR
        //  -   GL_ONE_MINUS_CONSTANT_COLOR
        //  -   GL_CONSTANT_ALPHA
        //  -   GL_ONE_MINUS_CONSTANT_ALPHA
        glBlendFunc(blend_source_factor, blend_destination_factor);
        // In case you're using any of the factors that use the constant color, you need to define it via the glBlendColor function.
        glBlendColor(blend_constant_color.r, blend_constant_color.g, blend_constant_color.b, blend_constant_color.a);

        // Enable alpha to coverage if needed.
        if(enable_alpha_to_coverage) glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        else glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

        glUseProgram(program);

        // If we use alpha testing, we need to send the threshold to the shader
        if(enable_alpha_test) program.set("alpha_threshold", alpha_test_threshold);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindSampler(0, sampler);
        program.set("sampler", 0);

        // Clear the render commands from the past frame then build them anew for the current frame.
        render_commands.clear();
        buildRenderCommands(root, camera.getVPMatrix());

        if(sort_render_commands)
            std::sort(std::begin(render_commands), std::end(render_commands));

        for(auto& render_command: render_commands){
            // If the object is transparent and we want to enable blending, we enable blending.
            if(render_command.transparent && enable_blending) glEnable(GL_BLEND);
            else glDisable(GL_BLEND);
            // For transparent objects, it is common to disable depth writing as an optimization since they're already sorted.
            glDepthMask(!render_command.transparent || enable_transparent_depth_write);
            glBindTexture(GL_TEXTURE_2D, render_command.texture);
            program.set("tint", render_command.tint);
            program.set("transform", render_command.transformation);
            render_command.mesh.lock()->draw();
        }
        glDepthMask(GL_TRUE);


    }

    void onDestroy() override {
        default_program.destroy();
        alpha_test_program.destroy();
        glDeleteSamplers(1, &sampler);
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

        ImGui::Checkbox("Enable Blending", &enable_blending);
        our::OptionMapCombo("Equation", blend_equation, our::gl_enum_options::blend_equations);
        our::OptionMapCombo("Source Function", blend_source_factor, our::gl_enum_options::blend_functions);
        our::OptionMapCombo("Destination Function", blend_destination_factor, our::gl_enum_options::blend_functions);
        ImGui::ColorEdit4("Blend Constant Color", glm::value_ptr(blend_constant_color), ImGuiColorEditFlags_HDR);

        ImGui::Separator();

        ImGui::Checkbox("Enable Sorting", &sort_render_commands);
        ImGui::TextWrapped("Sorting will render opaque objects first followed by transparent objects.");
        ImGui::TextWrapped("Opaque objects are renderer from nearest to farthest.");
        ImGui::TextWrapped("transparent objects are renderer from farthest to nearest.");

        ImGui::Separator();

        ImGui::Checkbox("Enable Alpha Testing", &enable_alpha_test);
        ImGui::DragFloat("Alpha Threshold", &alpha_test_threshold, 0.01f, 0.0f, 1.0f);

        ImGui::Separator();

        ImGui::Checkbox("Enable Alpha To Coverage", &enable_alpha_to_coverage);

        ImGui::Separator();

        ImGui::Checkbox("Enable Depth Testing", &enable_depth_test);
        our::OptionMapCombo("Comparison Function", depth_function, our::gl_enum_options::comparison_functions);
        ImGui::Checkbox("Enable Transparent Depth Write", &enable_transparent_depth_write);

        ImGui::Separator();

        ImGui::Checkbox("Enable Face Culling", &enable_face_culling);
        our::OptionMapCombo("Face To Cull", culled_face, our::gl_enum_options::facets);
        our::OptionMapCombo("Front Face", front_face_winding, our::gl_enum_options::face_windings);

        ImGui::End();


    }

};

int main(int argc, char** argv) {
    return BlendingApplication().run();
}
