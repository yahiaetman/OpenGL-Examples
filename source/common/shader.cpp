#include "shader.hpp"

#include <cassert>
#include <iostream>
#include <filesystem>

// Since GLSL doesn't support "#include" preprocessors, we use a library to do it for us called "stb_include"
#define STB_INCLUDE_LINE_GLSL
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
    // first, we use C++17 filesystem library to get the directory (parent) path of the file.
    // the parent path will be sent to stb_include to search for files referenced by any "#include" preprocessor command.
    auto file_path = std::filesystem::path(filename);
    auto file_path_string = file_path.string();
    auto parent_path_string = file_path.parent_path().string();
    auto path_to_includes = &(parent_path_string[0]);
    char error[256];

    // Read the file as a string and resolve any "#include"s recursively
    auto source = stb_include_file(&(file_path_string[0]), nullptr, path_to_includes, error);

    // Check if any loading errors happened
    if (source == nullptr) {
        std::cerr << "ERROR: " << error << std::endl;
        return false;
    }

    GLuint shaderID = glCreateShader(type); //Create shader of the given type

    // Function parameter:
    // shader (GLuint): shader object name.
    // count (GLsizei): number of strings passed in the third parameter. We only have one string here.
    // string (const GLchar**): an array of source code strings.
    // lengths (const GLint*): an array of string lengths for each string in the third parameter. if null is passed,
    //                          then the function will deduce the lengths automatically by searching for '\0'.
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
