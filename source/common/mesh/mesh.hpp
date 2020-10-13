#ifndef OUR_MESH_H
#define OUR_MESH_H

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <iostream>

#include <glad/gl.h>

#include "vertex-attributes.hpp"

namespace our {

    class Mesh {
    private:
        GLuint vertex_array = 0;
        GLuint element_buffer = 0;
        std::vector<GLuint> vertex_buffers;

        bool use_elements = false;
        size_t element_size = 1;
        GLenum element_type = GL_UNSIGNED_SHORT, primitive_mode = GL_TRIANGLES;
        GLsizei element_count = 0, vertex_count = 0;

    public:
        void create(const std::vector<std::function<void()>>& accessors, bool has_elements = true){
            vertex_buffers.resize(accessors.size());

            glGenVertexArrays(1, &vertex_array);
            glBindVertexArray(vertex_array);

            if(has_elements) {
                glGenBuffers(1, &element_buffer);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
                use_elements = true;
            }

            size_t buffer_count = accessors.size();
            glGenBuffers(buffer_count, vertex_buffers.data());
            for(size_t buffer_index = 0; buffer_index < buffer_count; ++buffer_index){
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[buffer_index]);
                accessors[buffer_index]();
            }

            glBindVertexArray(0);
        }

        [[nodiscard]] bool isCreated() const { return vertex_array != 0; }
        [[nodiscard]] bool hasElements() const { return element_buffer != 0; }
        [[nodiscard]] bool isUsingElements() const { return use_elements; }
        [[nodiscard]] GLenum getPrimitiveMode() const { return primitive_mode; }
        [[nodiscard]] GLsizei getElementCount() const { return element_count; }
        [[nodiscard]] GLsizei getVertexCount() const { return vertex_count; }

        void setUseElements(bool value){ use_elements = value && (element_buffer != 0); }
        void setElementCount(GLsizei value){ element_count = value; }
        void setVertexCount(GLsizei value){ vertex_count = value; }
        void setPrimitiveMode(GLenum mode){ primitive_mode = mode; }

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
        void setElementData(const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW){
            setElementData(data.data(), data.size(), usage);
        }

        template<typename T>
        void setElementData(T const * data, size_t count, GLenum usage = GL_STATIC_DRAW){
            if(element_buffer == 0) {
                std::cerr << "MESH ERROR: Setting element data before creating element buffer\n";
                return;
            }

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
        void getElementData(std::vector<T>& elements){
            assert(sizeof(T) == element_size);
            GLint size;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
            glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
            assert((size % sizeof(T)) == 0);
            elements.resize(size / sizeof(T));
            glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, elements.data());
        }

        template<typename T>
        std::vector<T>&& getElementData(){
            std::vector<T> elements;
            getElementData(elements);
            return elements;
        }

        template<typename T>
        void setVertexData(size_t buffer_index, const std::vector<T>& data, GLenum usage = GL_STATIC_DRAW){
            setVertexData(buffer_index, data.data(), data.size(), usage);
        }

        template<typename T>
        void setVertexData(size_t buffer_index, T const * data, size_t count, GLenum usage = GL_STATIC_DRAW){
            if(buffer_index >= vertex_buffers.size()) {
                std::cerr << "MESH ERROR: Setting vertex data to an out-of-bound vertex buffer (" << buffer_index << " >= " << vertex_buffers.size() << ")\n";
                return;
            }
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[buffer_index]);
            glBufferData(GL_ARRAY_BUFFER, count*sizeof(T), data, usage);
        }


        template<typename T>
        void setVertexSubData(size_t buffer_index, const std::vector<T>& data, GLintptr offset, GLenum usage = GL_STATIC_DRAW){
            setVertexSubData(buffer_index, data.data(), offset, data.size(), usage);
        }

        template<typename T>
        void setVertexSubData(size_t buffer_index, T const * data, GLintptr offset, size_t count, GLenum usage = GL_STATIC_DRAW){
            if(buffer_index >= vertex_buffers.size()) {
                std::cerr << "MESH ERROR: Setting vertex data to an out-of-bound vertex buffer (" << buffer_index << " >= " << vertex_buffers.size() << ")\n";
                return;
            }
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[buffer_index]);
            glBufferSubData(GL_ARRAY_BUFFER, offset, count*sizeof(T), data, usage);
        }

        template<typename T>
        void getVertexData(size_t buffer_index, std::vector<T>& vertices, GLintptr offset = 0, size_t count = 0){
            if(buffer_index >= vertex_buffers.size()) {
                std::cerr << "MESH ERROR: Requesting vertex data to an out-of-bound vertex buffer (" << buffer_index << " >= " << vertex_buffers.size() << ")\n";
                return;
            }
            GLint size;
            if(count == 0) {
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[buffer_index]);
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
                size -= offset;
                assert((size % sizeof(T)) == 0);
                vertices.resize(size / sizeof(T));
            } else {
                size = count * sizeof(T);
                vertices.resize(count);
            }
            glGetBufferSubData(GL_ARRAY_BUFFER, offset, size, vertices.data());
        }

        template<typename T>
        std::vector<T>&& getVertexData(size_t buffer_index, GLintptr offset = 0, size_t count = 0){
            std::vector<T> vertices;
            getVertexData<>(buffer_index, vertices, offset, count);
            return vertices;
        }

        void draw(GLsizei start = 0, GLsizei count = 0) const {
            if(use_elements) {
                const void *pointer = (void *) (element_size * start);
                if (count == 0) count = element_count;
                glBindVertexArray(vertex_array);
                glDrawElements(primitive_mode, count, element_type, pointer);
                glBindVertexArray(0);
            } else {
                if (count == 0) count = vertex_count;
                glBindVertexArray(vertex_array);
                glDrawArrays(primitive_mode, start, count);
                glBindVertexArray(0);
            }
        }

        //Delete copy constructor and assignment operation
        Mesh(Mesh const &) = delete;
        Mesh &operator=(Mesh const &) = delete;

    };

}

#endif //OUR_MESH_H
