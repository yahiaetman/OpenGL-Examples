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

    template<typename T>
    void setup_buffer_accessors() {
        static_assert(sizeof(T) != sizeof(T), "No accessors defined for this type");
    };
}

#endif //OUR_VERTEX_ATTRIBUTES_H
