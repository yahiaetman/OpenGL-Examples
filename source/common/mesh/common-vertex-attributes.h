#ifndef OUR_COMMON_VERTEX_ATTRIBUTES_H
#define OUR_COMMON_VERTEX_ATTRIBUTES_H

#include "vertex-attributes.h"
#include "common-vertex-types.h"

#include <initializer_list>

namespace our {

    template<>
    inline std::vector<VertexAttributeDefinition> define_buffer_accessors<ColoredVertex>() {
        return {
                {default_attribute_locations::POSITION, 3, GL_FLOAT, false, sizeof(ColoredVertex),
                        (void *) offsetof(ColoredVertex, position), false},
                {default_attribute_locations::COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(ColoredVertex),
                        (void *) offsetof(ColoredVertex, color), false}
        };
    };

    template<>
    inline std::vector<VertexAttributeDefinition> define_buffer_accessors<TexturedVertex>() {
        return {
                {default_attribute_locations::POSITION, 3, GL_FLOAT, false, sizeof(TexturedVertex),
                        (void *) offsetof(TexturedVertex, position), false},
                {default_attribute_locations::TEX_COORD, 2, GL_FLOAT, false, sizeof(TexturedVertex),
                        (void *) offsetof(TexturedVertex, tex_coord), false}
        };
    };

    template<>
    inline std::vector<VertexAttributeDefinition> define_buffer_accessors<Vertex>() {
        return {
            {default_attribute_locations::POSITION, 3, GL_FLOAT, false, sizeof(Vertex),
                    (void *) offsetof(Vertex, position), false},
            {default_attribute_locations::COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex),
                    (void *) offsetof(Vertex, color), false},
            {default_attribute_locations::TEX_COORD, 2, GL_FLOAT, false, sizeof(Vertex),
                    (void *) offsetof(Vertex, tex_coord), false},
            {default_attribute_locations::NORMAL, 3, GL_FLOAT, false, sizeof(Vertex),
                    (void *) offsetof(Vertex, normal), false}
        };
    };

}

#endif //OUR_COMMON_VERTEX_ATTRIBUTES_H
