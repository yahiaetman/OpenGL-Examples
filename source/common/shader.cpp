#include "shader.h"

#include <cassert>
#include <iostream>
#include <filesystem>

#define STB_INCLUDE_IMPLEMENTATION
#include <stb/stb_include.h>

void our::ShaderProgram::create() {
    //Create Shader Program
    program = glCreateProgram();
}

void our::ShaderProgram::destroy() {
    //Delete Shader Program
    if(program != 0) glDeleteProgram(program);
    program = 0;
}

bool our::ShaderProgram::attach(const std::string &filename, GLenum type) const {
    auto file_path = std::filesystem::path(filename);
    auto file_path_string = file_path.string();
    auto parent_path_string = file_path.parent_path().string();
    auto path_to_includes = &(parent_path_string[0]);
    char error[256];

    auto source = stb_include_file(&(file_path_string[0]), nullptr, path_to_includes, error);

    if (source == nullptr) {
        std::cerr << "ERROR: " << error << std::endl;
        return false;
    }

    GLuint shaderID = glCreateShader(type); //Create shader of the given type

    glShaderSource(shaderID, 1, &source, nullptr); //Send shader source code
    glCompileShader(shaderID); //Compile the shader code
    free(source);

    //Check and log for any error in the compilation process
    GLint status;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint length;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
        char *logStr = new char[length];
        glGetShaderInfoLog(shaderID, length, nullptr, logStr);
        std::cerr << "ERROR IN " << filename << std::endl;
        std::cerr << logStr << std::endl;
        delete[] logStr;
        glDeleteShader(shaderID);
        return false;
    }

    glAttachShader(program, shaderID); //Attach shader to program
    glDeleteShader(shaderID); //Delete shader (the shader is already attached to the program so its object is no longer needed)
    return true;
}

bool our::ShaderProgram::link() const {
    //Link
    glLinkProgram(program);

    //Check and log for any error in the linking process
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char *logStr = new char[length];
        glGetProgramInfoLog(program, length, nullptr, logStr);
        std::cerr << "LINKING ERROR" << std::endl;
        std::cerr << logStr << std::endl;
        delete[] logStr;
        return false;
    }
    return true;
}
