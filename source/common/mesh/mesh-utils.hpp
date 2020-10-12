#ifndef OUR_MESH_UTILS_H
#define OUR_MESH_UTILS_H

#include "mesh.hpp"

#include <glm/glm.hpp>

namespace our::mesh_utils {

    void loadOBJ(Mesh& mesh, const char* filename);
    void Cuboid(Mesh& mesh, bool colored_faces = false,
                const glm::vec3& center = {0,0,0},
                const glm::vec3& size = {1,1,1},
                const glm::vec2& texture_offset = {0, 0},
                const glm::vec2& texture_tiling = {1, 1});
    void Sphere(Mesh& mesh,
                const glm::ivec2& segments = {32, 16},
                bool colored = false,
                const glm::vec3& center = {0,0,0},
                float radius = 0.5f,
                const glm::vec2& texture_offset = {0, 0},
                const glm::vec2& texture_tiling = {1, 1});

}

#endif //OUR_MESH_UTILS_H
