#pragma once

#include <string>
#include <vector>
#include "renderer/vertex.hpp"

namespace slate {

    class MeshLoader {
    public:
        MeshLoader() = delete;

        static bool loadOBJ(
            const std::string& filePath,
            std::vector<Vertex>& outVertices,
            std::vector<uint16_t>& outIndices
        );
    };

}