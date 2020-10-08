#ifndef OUR_COMMON_VERTEX_ATTRIBUTES_H
#define OUR_COMMON_VERTEX_ATTRIBUTES_H

#include "vertex-attributes.h"
#include "common-vertex-types.h"

#include <glad/gl.h>

namespace our {

    template<>
    void setup_buffer_accessors<ColoredVertex>() {

        glEnableVertexAttribArray(default_attribute_locations::POSITION);
        glVertexAttribPointer(default_attribute_locations::POSITION, 3, GL_FLOAT, false, sizeof(ColoredVertex), (void*)offsetof(ColoredVertex, position));
        glEnableVertexAttribArray(default_attribute_locations::COLOR);
        glVertexAttribPointer(default_attribute_locations::COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(ColoredVertex), (void*)offsetof(ColoredVertex, color));

    };

    template<>
    void setup_buffer_accessors<TexturedVertex>() {

        glEnableVertexAttribArray(default_attribute_locations::POSITION);
        glVertexAttribPointer(default_attribute_locations::POSITION, 3, GL_FLOAT, false, sizeof(TexturedVertex), (void*)offsetof(TexturedVertex, position));
        glEnableVertexAttribArray(default_attribute_locations::TEX_COORD);
        glVertexAttribPointer(default_attribute_locations::TEX_COORD, 2, GL_FLOAT, false, sizeof(TexturedVertex), (void*)offsetof(TexturedVertex, tex_coord));

    };

    template<>
    void setup_buffer_accessors<Vertex>() {

        glEnableVertexAttribArray(default_attribute_locations::POSITION);
        glVertexAttribPointer(default_attribute_locations::POSITION, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(default_attribute_locations::COLOR);
        glVertexAttribPointer(default_attribute_locations::COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(default_attribute_locations::TEX_COORD);
        glVertexAttribPointer(default_attribute_locations::TEX_COORD, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));
        glEnableVertexAttribArray(default_attribute_locations::NORMAL);
        glVertexAttribPointer(default_attribute_locations::NORMAL, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    };

}

#endif //OUR_COMMON_VERTEX_ATTRIBUTES_H
