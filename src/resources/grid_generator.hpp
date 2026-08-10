#pragma once

#include "renderer/vertex.hpp"
#include <vector>
#include <glm/glm.hpp>

namespace slate {

    class GridGenerator {
    public:
        static void generatePlane(float size, std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices) {
            outVertices.clear();
            outIndices.clear();
            
            int divisions = 50;
            float step = (size * 2.0f) / divisions;

            for (int z = 0; z <= divisions; ++z) {
                for (int x = 0; x <= divisions; ++x) {
                    float px = -size + x * step;
                    float pz = -size + z * step;
                    outVertices.push_back({
                        glm::vec3(px, 0.0f, pz),
                        glm::vec3(1.0f),
                        glm::vec3(0, 1, 0),
                        glm::vec2(0)
                    });
                }
            }

            int stride = divisions + 1;
            for (int z = 0; z < divisions; ++z) {
                for (int x = 0; x < divisions; ++x) {
                    uint16_t i0 = z * stride + x;
                    uint16_t i1 = z * stride + (x + 1);
                    uint16_t i2 = (z + 1) * stride + (x + 1);
                    uint16_t i3 = (z + 1) * stride + x;

                    outIndices.push_back(i0);
                    outIndices.push_back(i1);
                    outIndices.push_back(i2);
                    outIndices.push_back(i0);
                    outIndices.push_back(i2);
                    outIndices.push_back(i3);
                }
            }
        }

        static void generateAxes(float size, std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices) {
            outVertices.clear();
            outIndices.clear();
        }
    };

}