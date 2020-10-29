#ifndef OUR_VERTEX_ATTRIBUTES_H
#define OUR_VERTEX_ATTRIBUTES_H

#include <glad/gl.h>

namespace our {

    // Just as convenience, we write which location "we" use for each attribute in our shader.
    // If you used different locations, modify these constants or just ignore (dont' use) them
    namespace default_attribute_locations {
        inline constexpr GLuint POSITION = 0;
        inline constexpr GLuint COLOR = 1;
        inline constexpr GLuint TEX_COORD = 2;
        inline constexpr GLuint NORMAL = 3;
    }

    // Also for convenience, we will specialize this function for every vertex struct we make to define how it should be sent to the attributes
    template<typename T>
    void setup_buffer_accessors() {
        // Make sure this is specialized for every type it is called for
        static_assert(sizeof(T) != sizeof(T), "No accessors defined for this type");
    };
}

#endif //OUR_VERTEX_ATTRIBUTES_H
