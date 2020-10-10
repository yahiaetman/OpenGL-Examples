#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <mesh/mesh.hpp>
#include <mesh/common-vertex-types.hpp>
#include <mesh/common-vertex-attributes.hpp>

#include <glm/gtx/euler_angles.hpp>

enum class TransformationType : int {
    Translation = 0,
    Rotation = 1,
    Scaling = 2
};

struct Transformation {
    TransformationType type;
    glm::vec3 value;
};

class CompositionApplication : public our::Application {

    our::ShaderProgram program;
    our::Mesh quad;

    std::vector<Transformation> transformations;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Transformation Matrix Composition", {1280, 720}, false };
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

    glm::mat4 compose(){
        glm::mat4 transformation_matrix(1.0f);
        for(const auto& transformation : transformations){
            switch (transformation.type) {
                case TransformationType::Translation:
                    transformation_matrix = glm::translate(glm::mat4(1.0f), transformation.value) * transformation_matrix;
                    break;
                case TransformationType::Rotation:
                    transformation_matrix = glm::yawPitchRoll(transformation.value.y, transformation.value.x, transformation.value.z) * transformation_matrix;
                    break;
                case TransformationType::Scaling:
                    transformation_matrix = glm::scale(glm::mat4(1.0f), transformation.value) * transformation_matrix;
                    break;
            }
        }
        return transformation_matrix;
    }

    void onDraw(double deltaTime) override {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);

        auto transformation_matrix = compose();
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

        ImGui::Text("Transformations");

        const char* transformation_names[] = {
                "Translation",
                "Rotation",
                "Scaling"
        };

        our::ReorderableList(transformations.begin(), transformations.end(),
                             [transformation_names](size_t index, Transformation& transform){
            auto selected = static_cast<int>(transform.type);
            if(ImGui::BeginCombo("Type", transformation_names[selected])){
                for(int selectable = 0; selectable < 3; ++selectable){
                    bool is_selected = selected == selectable;
                    if(ImGui::Selectable(transformation_names[selectable], is_selected))
                        selected = selectable;
                    if(is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
                transform.type = static_cast<TransformationType>(selected);
            }
            ImGui::DragFloat3("Value", glm::value_ptr(transform.value), 0.1f);
        }, [this](size_t index){
            transformations.insert(transformations.begin() + index, { TransformationType::Translation, glm::vec3() });
        }, [this](size_t index){
            transformations.erase(transformations.begin() + index);
        });

        if(ImGui::Button("Clear")){
            transformations.clear();
        }

        ImGui::Separator();

        ImGui::Text("Result:");
        auto matrix = compose();
        for(int row = 0; row < 4; ++row)
            ImGui::Text("%f\t%f\t%f\t%f", matrix[0][row], matrix[1][row], matrix[2][row], matrix[3][row]);

        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return CompositionApplication().run();
}
