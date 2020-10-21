#include <application.hpp>
#include <iostream>
#include <fstream>
#include <cassert>

#pragma region helper_functions

// Reads a file using a path and returns the file text in a string.
std::string read_file(const char* filename){
    std::ifstream fin(filename);
    if(fin.fail()){
        std::cerr << "Unable to open shader file: " << filename << std::endl;
        std::exit(-1);
    }
    return std::string(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());
}

void checkShaderCompilationErrors(GLuint shader){
    //Check and log for any error in the compilation process.
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);          // Takes a shader and returns the status of this shader program.
    if(!status){                                                // If there is a status:
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);     // Get the error length (char array length).      
        char* logStr = new char[length];
        glGetShaderInfoLog(shader, length, nullptr, logStr);    // Get the error char array.
        std::cerr << "ERROR:" << logStr << std::endl;           // print the char array of the log error.
        delete[] logStr;
        std::exit(-1);
    }
}

void checkProgramLinkingErrors(GLuint program){
    //Check and log for any error in the linking process.
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);               // Takes a shader program (vertex & fragment) and returns the status of this shader program.
    if (!status)                                                    // If there is a status:
    {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);       // Get the error length (char array length). 
        char* logStr = new char[length];
        glGetProgramInfoLog(program, length, nullptr, logStr);      // Get the error char array.
        std::cerr << "LINKING ERROR" << std::endl;
        std::cerr << logStr << std::endl;                           // print the char array of the log error.
        delete[] logStr;
        std::exit(-1);
    }
}

#pragma endregion


void attachShader(GLuint program, const char* filename, GLenum shader_type){
    // 1. Reads the shader from file, compiles it,
    std::string source_code = read_file(filename);
    const char* source_code_as_c_str = source_code.c_str();

    // 2. Pass the program as a string to the GPU and then compile it.
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source_code_as_c_str, nullptr);
    glCompileShader(shader);

    // 3. Check for any Compilation errors.
    checkShaderCompilationErrors(shader);

    // 4. Attach this shader to the program if no errors found in shader.
    glAttachShader(program, shader);

    // 5. Delete the shader as it is already attached to the program.
    glDeleteShader(shader);
}


// The ingerited class from "Application" to this example.
class ShaderIntroductionApplication : public our::Application {

    // These unsigned integers represent the way we communicate with the GPU.
    // They act like a pointer.
    // These uint are passed to the GL header functions to tell the GL what exactly are we
    // pointing at also the values are set by the GL functions when passed by refrence.
    GLuint program = 0, vertex_array = 0;

    // We need a window with title: "Shader Introduction", size: {1280, 720}, not full screen.
    our::WindowConfiguration getWindowConfiguration() override {
        return { "Shader Introduction", {1280, 720}, false };
    }

    void onInitialize() override {
        program = glCreateProgram();    // We ask GL to create a program for us and return a uint that we will use it by.
                                        // (act as a pointer to the created program).

        attachShader(program, "assets/shaders/ex02_shader_introduction/triangle.vert", GL_VERTEX_SHADER);   // read the vertex shader and attach it to the program.
        attachShader(program, "assets/shaders/ex02_shader_introduction/red.frag", GL_FRAGMENT_SHADER);      // read the fragment shader and attach it to the program.
        
        glLinkProgram(program);                     // Link the vertex and fragment shader together.
        checkProgramLinkingErrors(program);         // Check if there is any link errors between the fragment shader and vertex shader.

        glGenVertexArrays(1, &vertex_array);        // Ask GL to create a vertex array to easily create a triangle.

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void onDraw(double deltaTime) override {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);                      // Ask GL to use this program for the upcoming operations.
                                                    // Every shader and rendering call after glUseProgram will now use this program object (and the shaders).
        
        glBindVertexArray(vertex_array);            // Binding is like selecting which memory buffer to use.
        
        glDrawArrays(GL_TRIANGLES, 0, 3);           // Ask the GPU to draw the triangles.
        glBindVertexArray(0);                       // Unbind the buffer.
    }

    void onDestroy() override {
        glDeleteProgram(program);                   // Cleaning up the program we compiled and saved.
        glDeleteVertexArrays(1, &vertex_array);     // Clean up the array we allocated Params: (n: number of vertex array objects, array containing the n addresses of the objects)
    }

};

int main(int argc, char** argv) {
    return ShaderIntroductionApplication().run();
}
