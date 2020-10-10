#include <application.hpp>
#include <shader.hpp>


class UniformsApplication : public our::Application {

    our::ShaderProgram program;
    GLuint vertex_array = 0;

    glm::vec2 scale = glm::vec2(1,1);
    glm::vec2 translation = glm::vec2(0,0);
    glm::vec3 color = glm::vec3(1, 0, 0);
    bool vibrate = false, flicker = false;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Uniforms", {1280, 720}, false };
    }

    void onInitialize() override {
        program.create();
        program.attach("assets/shaders/ex03_uniforms/quad.vert", GL_VERTEX_SHADER);
        program.attach("assets/shaders/ex03_uniforms/uniform_color.frag", GL_FRAGMENT_SHADER);
        program.link();

        glGenVertexArrays(1, &vertex_array);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void onDraw(double deltaTime) override {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);

        GLuint scale_uniform_location = glGetUniformLocation(program, "scale");
        glUniform2f(scale_uniform_location, scale.x, scale.y);
        GLuint translation_uniform_location = glGetUniformLocation(program, "translation");
        glUniform2f(translation_uniform_location, translation.x, translation.y);
        GLuint color_uniform_location = glGetUniformLocation(program, "color");
        glUniform3f(color_uniform_location, color.r, color.g, color.b);

        GLuint time_uniform_location = glGetUniformLocation(program, "time");
        glUniform1f(time_uniform_location, glfwGetTime());
        GLuint vibrate_uniform_location = glGetUniformLocation(program, "vibrate");
        glUniform1i(vibrate_uniform_location, vibrate);
        GLuint flicker_uniform_location = glGetUniformLocation(program, "flicker");
        glUniform1i(flicker_uniform_location, flicker);


        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    void onDestroy() override {
        program.destroy();
        glDeleteVertexArrays(1, &vertex_array);
    }

    void onImmediateGui(ImGuiIO &io) override {
        ImGui::Begin("Controls");
        ImGui::SliderFloat2("Scale", glm::value_ptr(scale), 0, 1);
        ImGui::SliderFloat2("Translation", glm::value_ptr(translation), -2, 2);
        ImGui::ColorEdit3("Color", glm::value_ptr(color));
        ImGui::Checkbox("Vibrate", &vibrate);
        ImGui::Checkbox("Flicker", &flicker);
        ImGui::Value("Time: ", (float)glfwGetTime());
        ImGui::End();
    }

};

int main(int argc, char** argv) {
    return UniformsApplication().run();
}
