#ifndef OUR_MESH_H
#define OUR_MESH_H

#include <map>
#include <string>
#include <vector>
#include <functional>

#include <glad/gl.h>

#include "vertex-attributes.h"

namespace our {

    class Mesh {
    private:
        GLuint vertex_array = 0;
        GLuint element_buffer = 0;
        std::vector<GLuint> vertex_buffers;

        size_t element_size = 1;
        GLenum element_type = GL_UNSIGNED_SHORT;
        GLsizei element_count = 0;

    public:
        void create(const std::vector<std::function<void()>>& accessors){
            vertex_buffers.resize(accessors.size());

            glGenVertexArrays(1, &vertex_array);
            glBindVertexArray(vertex_array);

            glGenBuffers(1, &element_buffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);

            size_t buffer_count = accessors.size();
            glGenBuffers(buffer_count, vertex_buffers.data());
            for(size_t buffer_index = 0; buffer_index < buffer_count; ++buffer_index){
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[buffer_index]);
                accessors[buffer_index]();
            }

            glBindVertexArray(0);
        }

        [[nodiscard]] bool isCreated() const { return vertex_array != 0; }

        void destroy(){
            if(vertex_array != 0) glDeleteVertexArrays(1, &vertex_array);
            vertex_array = 0;
            if(element_buffer != 0) glDeleteBuffers(1, &element_buffer);
            element_buffer = 0;
            glDeleteBuffers(vertex_buffers.size(), vertex_buffers.data());
            vertex_buffers.resize(0);
        }

        Mesh() = default;
        ~Mesh(){ destroy(); }

        template<typename T>
        void setElementData(const std::vector<T>& data, GLenum usage){
            setElementData(data.data(), data.size(), usage);
        }

        template<typename T>
        void setElementData(T const * data, size_t count, GLenum usage){
            if(element_buffer == 0) return;

            element_size = sizeof(T);

            if constexpr (sizeof(T) == 4) element_type = GL_UNSIGNED_INT;
            else if constexpr (sizeof(T) == 2) element_type = GL_UNSIGNED_SHORT;
            else if constexpr (sizeof(T) == 2) element_type = GL_UNSIGNED_BYTE;
            else static_assert(sizeof(T) != sizeof(T), "Unsupported Element type size");

            element_count = count;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_count * element_size, data, usage);
        }

        template<typename T>
        void setVertexData(size_t buffer_index, const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW){
            setVertexData(buffer_index, data.data(), data.size(), usage);
        }

        template<typename T>
        void setVertexData(size_t buffer_index, T const * data, size_t count, GLenum usage = GL_STATIC_DRAW){
            if(buffer_index >= vertex_buffers.size()) return;
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[buffer_index]);
            glBufferData(GL_ARRAY_BUFFER, count*sizeof(T), data, usage);
        }

        void draw(GLenum mode = GL_TRIANGLES, GLsizei start = 0, GLsizei count = 0) const {
            const void* pointer = (void*)(element_size * start);
            if(count == 0) count = element_count;
            glBindVertexArray(vertex_array);
            glDrawElements(mode, count, element_type, pointer);
            glBindVertexArray(0);
        }

        //Delete copy constructor and assignment operation
        Mesh(Mesh const &) = delete;
        Mesh &operator=(Mesh const &) = delete;

        //Utility Functions
        static void loadOBJ(Mesh& mesh, const char* filename);

    };

}

#endif //OUR_MESH_H
