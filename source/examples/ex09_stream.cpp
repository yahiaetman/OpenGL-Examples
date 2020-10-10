#include <application.hpp>
#include <shader.hpp>
#include <imgui-utils/utils.hpp>

#include <vector>


struct Vertex {
    glm::vec3 position;
    glm::vec<4, glm::uint8, glm::defaultp> color;
};

class ElementsApplication : public our::Application {

    our::ShaderProgram program;
    GLuint vertex_array = 0, vertex_buffer = 0, element_buffer = 0;

    std::vector<Vertex> vertices = {
        {{-0.5, -0.5, 0.0},{255, 0, 0, 255}},
        {{0.5, -0.5, 0.0},{0, 255, 0, 255}},
        {{0.0, 0.5, 0.0},{0, 0, 255, 255}}
    };
    std::vector<uint16_t> elements = { 0, 1, 2 };

    GLenum primitive_mode = GL_TRIANGLES, polygon_mode = GL_FILL;
    bool use_elements = true;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Stream", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex06_multiple_attributes/multiple_attributes.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex04_varyings/varying_color.frag", GL_FRAGMENT_SHADER);
        program.link();

        glGenVertexArrays(1, &vertex_array);
        glBindVertexArray(vertex_array);

        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STREAM_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &element_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size()*sizeof(uint16_t), elements.data(), GL_STREAM_DRAW);

        glBindVertexArray(0);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void onDraw(double deltaTime) override {
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_STREAM_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size()*sizeof(uint16_t), elements.data(), GL_STREAM_DRAW);

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(vertex_array);
        glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
        if(use_elements)
            glDrawElements(primitive_mode, elements.size(), GL_UNSIGNED_SHORT, (void*)0);
        else
            glDrawArrays(primitive_mode, 0, vertices.size());
        glBindVertexArray(0);

        if(keyboard.justPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);
    }

    void onDestroy() override {
        program.destroy();
        glDeleteVertexArrays(1, &vertex_array);
        glDeleteBuffers(1, &vertex_buffer);;
        glDeleteBuffers(1, &element_buffer);
    }

    void onImmediateGui(ImGuiIO &io) override {

        ImGui::Begin("Controls");
        our::OptionMapCombo("Primitive Type", primitive_mode, our::gl_enum_options::primitives);
        our::OptionMapCombo("Polygon Mode", polygon_mode, our::gl_enum_options::polygon_modes);
        ImGui::Checkbox("Use Elements", &use_elements);
        ImGui::End();

        ImGui::Begin("Vertices");
        our::ReorderableList(std::begin(vertices), std::end(vertices),
        [](size_t index, Vertex& vertex){
           ImGui::Text("Vertex %zu", index);
           ImGui::DragFloat3("Position", glm::value_ptr(vertex.position), 0.01, -2, 2);
           our::ColorEdit4U8("Color", glm::value_ptr(vertex.color));
        },
        [&](size_t index){ vertices.insert(std::begin(vertices) + index, Vertex()); },
        [&](size_t index){ vertices.erase(std::begin(vertices) + index); });
        ImGui::End();

        ImGui::Begin("Elements");
        int max_element = (int)vertices.size() - 1;
        float speed = 1.0f / (float)(max_element + 1);
        our::ReorderableList(std::begin(elements), std::end(elements),
        [&](size_t index, uint16_t & element){
            std::string str_id = std::to_string(index);
            int element_i32 = element;
            ImGui::DragInt(str_id.c_str(), &element_i32, speed, 0, max_element);
            element = element_i32;
        },
        [&](size_t index){ elements.insert(std::begin(elements) + index, 0); },
        [&](size_t index){ elements.erase(std::begin(elements) + index); });
        ImGui::End();

    }

};

int main(int argc, char** argv) {
    return ElementsApplication().run();
}
