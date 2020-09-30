#include <application.h>
#include <iostream>
#include <fstream>
#include <cassert>

#pragma region helper_functions

std::string read_file(const char* filename){
    std::ifstream fin(filename);
    if(fin.fail()){
        std::cerr << "Unable to open shader file: " << filename << std::endl;
        std::exit(-1);
    }
    return std::string(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>());
}

void checkShaderCompilationErrors(GLuint shader){
    //Check and log for any error in the compilation process
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(!status){
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char* logStr = new char[length];
        glGetShaderInfoLog(shader, length, nullptr, logStr);
        std::cerr << "ERROR:" << logStr << std::endl;
        delete[] logStr;
        std::exit(-1);
    }
}

void checkProgramLinkingErrors(GLuint program){
    //Check and log for any error in the linking process
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status)
    {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char* logStr = new char[length];
        glGetProgramInfoLog(program, length, nullptr, logStr);
        std::cerr << "LINKING ERROR" << std::endl;
        std::cerr << logStr << std::endl;
        delete[] logStr;
        std::exit(-1);
    }
}

#pragma endregion


void attachShader(GLuint program, const char* filename, GLenum shader_type){

    std::string source_code = read_file(filename);
    const char* source_code_as_c_str = source_code.c_str();

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source_code_as_c_str, nullptr);
    glCompileShader(shader);

    checkShaderCompilationErrors(shader);

    glAttachShader(program, shader);
    glDeleteShader(shader);
}


class ShaderIntroductionApplication : public our::Application {

    GLuint program = 0, vertex_array = 0;

    our::WindowConfiguration getWindowConfiguration() override {
        return { "Shader Introduction", {1280, 720}, false };
    }

    void onInitialize() override {
        program = glCreateProgram();
        attachShader(program, "assets/shaders/ex02_shader_introduction/triangle.vert", GL_VERTEX_SHADER);
        attachShader(program, "assets/shaders/ex02_shader_introduction/red.frag", GL_FRAGMENT_SHADER);
        glLinkProgram(program);
        checkProgramLinkingErrors(program);

        glGenVertexArrays(1, &vertex_array);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void onDraw(double deltaTime) override {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    }

    void onDestroy() override {
        glDeleteProgram(program);
        glDeleteVertexArrays(1, &vertex_array);
    }

};

int main(int argc, char** argv) {
    return ShaderIntroductionApplication().run();
}
