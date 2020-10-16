#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <map>

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace our {

    class ShaderProgram {

    private:
        //Shader Program Handle
        GLuint program;
        std::map<std::string, GLuint> uniform_location_cache;

    public:
        void create();
        void destroy();

        ShaderProgram(){ program = 0; }
        ~ShaderProgram(){ destroy(); }

        //Cast Class to an OpenGL Object name
        operator GLuint() const { return program; } // NOLINT: Allow implicit casting for convenience

        //Read shader from file, send it to GPU, compile it then attach it to shader
        bool attach(const std::string &filename, GLenum type) const; // NOLINT: attach does alter the object state so [[nodiscard]] is unneeded

        //Link Program (Do this after all shaders are attached)
        bool link() const; // NOLINT: link does alter the object state so [[nodiscard]] is unneeded

        //Get the location of a uniform variable in the shader
        GLuint getUniformLocation(const std::string &name) {
            auto it = uniform_location_cache.find(name);
            if(it != uniform_location_cache.end()){
                return it->second;
            }
            GLuint location = glGetUniformLocation(program, name.c_str());
            uniform_location_cache[name] = location;
            return location;
        }

        //A group of setter for uniform variables
        //NOTE: It is inefficient to call glGetUniformLocation every frame
        //So it is usually a better option to either cache the location
        //or explicitly define the uniform location in the shader
        void set(const std::string &uniform, GLfloat value) {
            glUniform1f(getUniformLocation(uniform), value);
        }

        void set(const std::string &uniform, GLint value) {
            glUniform1i(getUniformLocation(uniform), value);
        }

        void set(const std::string &uniform, GLboolean value) {
            glUniform1i(getUniformLocation(uniform), value);
        }

        void set(const std::string &uniform, glm::vec2 value) {
            glUniform2f(getUniformLocation(uniform), value.x, value.y);
        }

        void set(const std::string &uniform, glm::vec3 value) {
            glUniform3f(getUniformLocation(uniform), value.x, value.y, value.z);
        }

        void set(const std::string &uniform, glm::vec4 value) {
            glUniform4f(getUniformLocation(uniform), value.x, value.y, value.z, value.w);
        }

        void set(const std::string &uniform, glm::mat4 value, GLboolean transpose = false)  {
            glUniformMatrix4fv(getUniformLocation(uniform), 1, transpose, glm::value_ptr(value));
        }


        //Delete copy constructor and assignment operation
        ShaderProgram(ShaderProgram const &) = delete;
        ShaderProgram &operator=(ShaderProgram const &) = delete;
    };

}

#endif