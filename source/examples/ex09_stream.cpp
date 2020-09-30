#include <application.h>
#include <shader.h>
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

    GLenum primitive_type = GL_TRIANGLES;
    bool use_elements = true;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Interleaved Attributes", {1280, 720}, false };
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
        if(use_elements)
            glDrawElements(primitive_type, elements.size(), GL_UNSIGNED_SHORT, (void*)0);
        else
            glDrawArrays(primitive_type, 0, vertices.size());
        glBindVertexArray(0);
    }

    void onDestroy() override {
        program.destroy();
        glDeleteVertexArrays(1, &vertex_array);
        glDeleteBuffers(1, &vertex_buffer);;
        glDeleteBuffers(1, &element_buffer);
    }

    void onImmediateGui(ImGuiIO &io) override {
        GLenum primitives[] = {
          GL_POINTS,
          GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP,
          GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN
        };
        auto primitive_name = [](GLenum primitive) -> const char* {
            switch (primitive) {
                case GL_POINTS: return "GL_POINTS";
                case GL_LINES: return "GL_LINES";
                case GL_LINE_STRIP: return "GL_LINE_STRIP";
                case GL_LINE_LOOP: return "GL_LINE_LOOP";
                case GL_TRIANGLES: return "GL_TRIANGLES";
                case GL_TRIANGLE_STRIP: return "GL_TRIANGLE_STRIP";
                case GL_TRIANGLE_FAN: return "GL_TRIANGLE_FAN";
                default: return "";
            }
        };
        ImGui::Begin("Controls");
        if(ImGui::BeginCombo("Primitive Type", primitive_name(primitive_type))){
            for(GLenum primitive : primitives){
                bool is_selected = primitive_type == primitive;
                if(ImGui::Selectable(primitive_name(primitive), is_selected))
                    primitive_type = primitive;
                if(is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::Checkbox("Use Elements", &use_elements);
        ImGui::End();

        int deleted_item = -1, moved_item = -1, move_direction = 0;
        ImGui::Begin("Vertices");
        for (uint16_t index = 0, size = vertices.size(); index < size; ++index) {
            auto str_id = "Vertex " + std::to_string(index);
            ImGui::PushID(str_id.c_str());
            Vertex &vertex = vertices[index];
            ImGui::Text(str_id.c_str());
            ImGui::DragFloat3("Position", glm::value_ptr(vertex.position), 0.01, -2, 2);
            glm::vec4 color = glm::vec4(vertex.color) / 255.0f;
            ImGui::ColorEdit4("Color", glm::value_ptr(color));
            vertex.color = color * 255.0f;
            if(ImGui::Button("-"))
                deleted_item = index;
            ImGui::SameLine();
            if(index > 0 && ImGui::Button("^")){
                move_direction = -1;
                moved_item = index;
            }
            ImGui::SameLine();
            if(index < size - 1 && ImGui::Button("v")){
                move_direction = +1;
                moved_item = index;
            }
            ImGui::SameLine();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::PopID();
        }
        bool add = ImGui::Button("+");
        ImGui::End();
        if(add){
            vertices.emplace_back();
        } else if(deleted_item != -1){
            vertices.erase(vertices.begin() + deleted_item);
        } else if(moved_item != -1){
            int other = moved_item + move_direction;
            Vertex temp = vertices[moved_item];
            vertices[moved_item] = vertices[other];
            vertices[other] = temp;
        }

        deleted_item = -1, moved_item = -1, move_direction = 0;
        ImGui::Begin("Elements");
        int max_element = (int)vertices.size() - 1;
        for (uint16_t index = 0, size = elements.size(); index < size; ++index) {
            auto str_id = "Element " + std::to_string(index);
            ImGui::PushID(str_id.c_str());
            int element = elements[index];
            ImGui::DragInt(str_id.c_str(), &element, 1.0f/(max_element+1), 0, max_element);
            elements[index] = element;
            if(ImGui::Button("-"))
                deleted_item = index;
            ImGui::SameLine();
            if(index > 0 && ImGui::Button("^")){
                move_direction = -1;
                moved_item = index;
            }
            ImGui::SameLine();
            if(index < size - 1 && ImGui::Button("v")){
                move_direction = +1;
                moved_item = index;
            }
            ImGui::SameLine();
            ImGui::PopID();
            ImGui::NewLine();
        }
        add = ImGui::Button("+");
        ImGui::End();
        if(add){
            elements.emplace_back();
        } else if(deleted_item != -1){
            elements.erase(elements.begin() + deleted_item);
        } else if(moved_item != -1){
            int other = moved_item + move_direction;
            uint16_t temp = elements[moved_item];
            elements[moved_item] = elements[other];
            elements[other] = temp;
        }


    }

};

int main(int argc, char** argv) {
    return ElementsApplication().run();
}
