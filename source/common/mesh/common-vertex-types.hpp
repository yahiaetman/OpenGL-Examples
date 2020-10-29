#ifndef OUR_VERTEX_TYPES_H
#define OUR_VERTEX_TYPES_H

#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtx/hash.hpp>

#include <data-types.h>

namespace our {

    // Some Vertex type that we will "probably" use

    // A vertex with a position and a color only
    struct ColoredVertex {
        glm::vec3 position;
        Color color;
    };

    // A vertex with a position and a texture coordinate only
    struct TexturedVertex {
        glm::vec3 position;
        glm::vec2 tex_coord;
    };

    // A vertex with a position, a color, a texture coordinate and a surface normal
    struct Vertex {
        glm::vec3 position;
        Color color;
        glm::vec2 tex_coord;
        glm::vec3 normal;

        // We plan to use this as a key for a map so we need to define the equality operator
        bool operator==(const Vertex& other) const {
            return position == other.position &&
                   color == other.color &&
                   tex_coord == other.tex_coord &&
                   normal == other.normal;
        }
    };

}


// We plan to use struct Vertex as a key for a map so we need to define a hash function for it
namespace std {
    //A Simple method to combine two hash values
    inline size_t hash_combine(size_t h1, size_t h2){ return h1 ^ (h2 << 1); }

    //A Hash function for struct Vertex
    template<> struct hash<our::Vertex> {
        size_t operator()(our::Vertex const& vertex) const {
            size_t combined = hash<glm::vec3>()(vertex.position);
            combined = hash_combine(combined, hash<our::Color>()(vertex.color));
            combined = hash_combine(combined, hash<glm::vec2>()(vertex.tex_coord));
            combined = hash_combine(combined, hash<glm::vec3>()(vertex.normal));
            return combined;
        }
    };
}

#endif //OUR_VERTEX_TYPES_H
