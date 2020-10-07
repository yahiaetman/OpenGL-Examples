#include "single-buffer-mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj/tiny_obj_loader.h>

#include <iostream>
#include <vector>
#include <unordered_map>

#include "common-vertex-types.h"
#include "common-vertex-attributes.h"

void our::Mesh::loadOBJ(our::Mesh &mesh, const char* filename) {
    std::vector<our::Vertex> vertices;
    std::vector<GLuint> elements;
    std::unordered_map<our::Vertex, GLuint> vertex_map;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename)) {
        throw std::runtime_error(err);
    }
    if (!warn.empty()) {
        std::cout << "WARN while loading obj file \"" << filename << "\": " << warn << std::endl;
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};

            vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
            };

            vertex.tex_coord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
            };


            vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 0],
                    1.0f
            };

            auto it = vertex_map.find(vertex);
            if(it == vertex_map.end()){
                auto new_vertex_index = static_cast<GLuint>(vertices.size());
                vertex_map[vertex] = new_vertex_index;
                elements.push_back(new_vertex_index);
                vertices.push_back(vertex);
            } else {
                elements.push_back(it->second);
            }

            if(mesh.isCreated()) mesh.destroy();
            mesh.create({
                our::define_buffer_accessors<Vertex>()
            });
            mesh.setElementData(elements, GL_STATIC_DRAW);
            mesh.setVertexData(0, vertices, GL_STATIC_DRAW);
        }
    }

}
