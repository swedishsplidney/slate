#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "resources/mesh_loader.hpp"
#include <iostream>

namespace slate {

    bool MeshLoader::loadOBJ(
        const std::string& filePath,
        std::vector<Vertex>& outVertices,
        std::vector<uint16_t>& outIndices
    ) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filePath.c_str(), nullptr, true)) {
            std::cerr << "tinyobjloader error: " << err << std::endl;
            return false;
        }

        if (!err.empty()) {
            std::cout << "tinyobjloader messages: " << err << std::endl;
        }

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                // read positions explicitly
                glm::vec3 position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                // fallback
                glm::vec3 color = {0.8f, 0.8f, 0.8f};

                // construct vertex
                Vertex vertex(position, color);

                // push back linearly
                outVertices.push_back(vertex);
                outIndices.push_back(static_cast<uint32_t>(outIndices.size() - 1));
            }
        }

        std::cout << "successfully loaded obj: " << filePath << " (" << outVertices.size() << " vertices)" << std::endl;

        return true;
    }

}