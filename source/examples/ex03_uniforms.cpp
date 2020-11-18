#include <application.hpp>
#include <shader.hpp>

// The inherited class from "Application" to this example.
class UniformsApplication : public our::Application {

    our::ShaderProgram program;         // Instance of our shader class.
    GLuint vertex_array = 0;            // The address of the vertex_array saved on the VRAM.

    glm::vec2 scale = glm::vec2(1,1);           // This vector of size 2 represents scale(size).
    glm::vec2 translation = glm::vec2(0,0);     // This vector of size 2 represents translation(position).
    glm::vec3 color = glm::vec3(1, 0, 0);       // This vector of size 3 represents color.
    bool vibrate = false, flicker = false;

    // We need a window with title: "Uniforms", size: {1280, 720}, not full screen.
    our::WindowConfiguration getWindowConfiguration() override {
        return { "Uniforms", {1280, 720}, false };
    }

    // Remember called once before game loop.
    void onInitialize() override {
        program.create();                                                                               // Create a "Program".
        program.attach("assets/shaders/ex03_uniforms/quad.vert", GL_VERTEX_SHADER);                     // Read vertex shader file, compile it, and check for errors.
        program.attach("assets/shaders/ex03_uniforms/uniform_color.frag", GL_FRAGMENT_SHADER);          // Read fragment shader file, compile it, and check for errors.
        program.link();                                                                                 // link to shaders together in the program.

        glGenVertexArrays(1, &vertex_array);            // Create a vertex array and save address in uint vertex array.

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);           // Set the screen clear color to black.
    }

    // Remember called in game loop phase every frame.
    void onDraw(double deltaTime) override {
        
        glClear(GL_COLOR_BUFFER_BIT);               // Clear the frame.
        glUseProgram(program);                      // Set "program" as the GPU drawing program.


        // Set the GPU parameters every frame as the position, scale, color are not permenant values,
        // they change every frame, unlike the vertex data for example that needs to be set once before
        // application starts.
        GLuint scale_uniform_location = glGetUniformLocation(program, "scale");                 // Get the address to the program variable "scale" found
                                                                                                // in vertex shader "quad.vert file".
        glUniform2f(scale_uniform_location, scale.x, scale.y);                                  // Set the value of vec2 "scale_uniform_location" with scale data.
        
        
        GLuint translation_uniform_location = glGetUniformLocation(program, "translation");     // Get the address to the program variable "translation" found
                                                                                                // in vertex shader "quad.vert file".
        glUniform2f(translation_uniform_location, translation.x, translation.y);                // Set the value of vec2 "translation_uniform_location" with translation data.
        
        
        GLuint color_uniform_location = glGetUniformLocation(program, "color");                 // Get the address to the program variable "color" found
                                                                                                // in fragment shader "uniform_color.frag file".
        glUniform3f(color_uniform_location, color.r, color.g, color.b);                         // Set the value of vec3 "color_uniform_location" with color data.

        GLuint time_uniform_location = glGetUniformLocation(program, "time");
        glUniform1f(time_uniform_location, glfwGetTime());

        GLuint vibrate_uniform_location = glGetUniformLocation(program, "vibrate");
        glUniform1i(vibrate_uniform_location, vibrate);

        GLuint flicker_uniform_location = glGetUniformLocation(program, "flicker");
        glUniform1i(flicker_uniform_location, flicker);


        glBindVertexArray(vertex_array);        // Bind vertext array (select the buffer to be used by program).
        
        glDrawArrays(GL_TRIANGLES, 0, 6);       // Use the buffer and selected program to draw.
                                                // 1st parameter: Specifies what kind of primitives to render.
                                                // 2nd parameter: Specifies the starting index in the enabled arrays.
                                                // 3rd parameter: Specifies the number of indices to be rendered.
                                                // 3rd parameter is 6 as we draw 2 triangles (each with 3 vertices).

        glBindVertexArray(0);                   // Unbind the array(buffer).
    }

    void onDestroy() override {
        program.destroy();                              // Delete the program.
        glDeleteVertexArrays(1, &vertex_array);         // Free allocated memory.
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
