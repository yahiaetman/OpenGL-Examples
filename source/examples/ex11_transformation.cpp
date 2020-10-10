#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <mesh/common-vertex-types.hpp>
#include <mesh/common-vertex-attributes.hpp>

class TransformationApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh quad;

    glm::mat4 transformation_matrix = glm::mat4(1.0f);

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Transformation Matrix", {1280, 720}, false };
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

        glClearColor(0, 0, 0, 0);
    }

    void onDraw(double deltaTime) override {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);

        program.set("transform", transformation_matrix);
        program.set("tint", glm::vec4(1,1,1,1));

        quad.draw();
    }

    void onDestroy() override {
        program.destroy();
        quad.destroy();
    }

    void onImmediateGui(ImGuiIO &io) override {

        ImGui::Begin("Controls");

        ImGui::Text("Transformation Matrix");

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
        for(uint8_t row = 0; row < 4; ++row){
            for(uint8_t column = 0; column < 4; ++column){
                ImGui::PushID((row << 2) + column);
                ImGui::DragFloat("", &(transformation_matrix[column][row]),0.1f);
                ImGui::SameLine(0, 0);
                ImGui::PopID();
            }
            ImGui::NewLine();
        }

        if(ImGui::Button("Reset")){
            transformation_matrix = glm::mat4(1.0f);
        }

        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return TransformationApplication().run();
}
