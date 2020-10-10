#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <mesh/common-vertex-types.hpp>
#include <mesh/mesh-utils.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec<4, glm::uint8, glm::defaultp> color;
};

class MeshApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh quad, model;

    glm::vec4 clear_color;
    uint8_t mesh_to_render_index;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Model Loading", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex06_multiple_attributes/multiple_attributes.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex04_varyings/varying_color.frag", GL_FRAGMENT_SHADER);
        program.link();

        quad.create({
            [](){
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), (void*)offsetof(Vertex, color));
            }
        });
        quad.setVertexData<Vertex>(0, {
           {{-0.5, -0.5, 0},{255,   0,   0, 255}},
           {{ 0.5, -0.5, 0},{  0, 255,   0, 255}},
           {{ 0.5,  0.5, 0},{  0,   0, 255, 255}},
           {{-0.5,  0.5, 0},{255, 255,   0, 255}}
        },GL_STATIC_DRAW);
        quad.setElementData<GLuint>({
            0, 1, 2,
            2, 3, 0
        },GL_STATIC_DRAW);

        our::mesh_utils::loadOBJ(model, "assets/models/Suzanne/Suzanne.obj");

    }

    void onDraw(double deltaTime) override {
        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        switch (mesh_to_render_index) {
            case 0:
                quad.draw();
                break;
            case 1:
                model.draw();
                break;
        }

    }

    void onDestroy() override {
        program.destroy();
        quad.destroy();
        model.destroy();
    }

    void onImmediateGui(ImGuiIO &io) override {

        ImGui::Begin("Controls");

        const char* model_names[] = {
                "Quad",
                "Model"
        };
        const size_t model_count = sizeof(model_names)/sizeof(model_names[0]);

        if(ImGui::BeginCombo("Mesh", model_names[mesh_to_render_index])){
            for(size_t index = 0; index < model_count; ++index){
                bool is_selected = mesh_to_render_index == index;
                if(ImGui::Selectable(model_names[index], is_selected))
                    mesh_to_render_index = index;
                if(is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::ColorEdit4("Clear Color", glm::value_ptr(clear_color));

        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return MeshApplication().run();
}
