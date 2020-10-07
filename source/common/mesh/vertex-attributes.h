#ifndef OUR_VERTEX_ATTRIBUTES_H
#define OUR_VERTEX_ATTRIBUTES_H

#include <glad/gl.h>

namespace our {

    namespace default_attribute_locations {
        inline constexpr GLuint POSITION = 0;
        inline constexpr GLuint COLOR = 1;
        inline constexpr GLuint TEX_COORD = 2;
        inline constexpr GLuint NORMAL = 3;
    }

    struct VertexAttributeDefinition {
        GLuint attribute_location;
        GLint size;
        GLenum type;
        GLboolean normalized;
        GLsizei stride;
        const void* pointer;
        bool is_integer;

        static constexpr VertexAttributeDefinition define(
                GLuint location, GLint size, GLenum type, GLboolean normalized = GL_FALSE,
                GLsizei stride = 0, const void * pointer = nullptr, bool is_integer = false
                ) {
            return { location, size, type, normalized, stride, pointer, is_integer };
        }
    };

    template<typename T>
    inline std::vector<VertexAttributeDefinition> define_buffer_accessors() {
        static_assert(sizeof(T) != sizeof(T), "No accessors defined for this type");
        return {};
    };

    template<>
    inline std::vector<VertexAttributeDefinition> define_buffer_accessors<float>() {
        return {
            VertexAttributeDefinition::define(default_attribute_locations::POSITION, 3, GL_FLOAT)
        };
    };
}

#endif //OUR_VERTEX_ATTRIBUTES_H
