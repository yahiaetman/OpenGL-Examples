#include <application.hpp>
#include <shader.hpp>

// The inherited class from "Application" to this example.
class VaryingsApplication : public our::Application {

    our::ShaderProgram program;         // Instance of our shader class.
    GLuint vertex_array = 0;            // The address of the vertex_array saved on the VRAM.

    // We need a window with title: "Varyings", size: {1280, 720}, not full screen.
    our::WindowConfiguration getWindowConfiguration() override {
        return { "Varyings", {1280, 720}, false };
    }

    // Remember called once before game loop.
    void onInitialize() override {
        program.create();                                                                               // Create a "Program".
        program.attach("assets/shaders/ex04_varyings/colored_triangle.vert", GL_VERTEX_SHADER);         // Read vertex shader file, compile it, and check for errors.
        program.attach("assets/shaders/ex04_varyings/varying_color.frag", GL_FRAGMENT_SHADER);          // Read fragment shader file, compile it, and check for errors.
        program.link();                                                                                 // link to shaders together in the program.

        glGenVertexArrays(1, &vertex_array);            // Create a vertex array and save address in uint vertex array.

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);           // Set the screen clear color to black.
    }

    // Remember called in game loop phase every frame.
    void onDraw(double deltaTime) override {

        glClear(GL_COLOR_BUFFER_BIT);                   // Clear the frame.
        glUseProgram(program);                          // Set "program" as the GPU drawing program.
        glBindVertexArray(vertex_array);                // Bind vertext array (select the buffer to be used by program).
        
        glDrawArrays(GL_TRIANGLES, 0, 3);               // Use the buffer and selected program to draw.
                                                        // 1st parameter: Specifies what kind of primitives to render.
                                                        // 2nd parameter: Specifies the starting index in the enabled arrays.
                                                        // 3rd parameter: Specifies the number of indices to be rendered.
                                                        // 3rd parameter is 3 as we draw 1 triangles (each with 3 vertices).
        
        glBindVertexArray(0);                           // Unbind the array(buffer).
    }   

    void onDestroy() override {
        program.destroy();                              // Delete the program.
        glDeleteVertexArrays(1, &vertex_array);         // Free allocated memory.
    }

};

int main(int argc, char** argv) {
    return VaryingsApplication().run();
}
