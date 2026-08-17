#pragma once

#include "renderer/vertex.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <cmath>

namespace slate {

    class GizmoGenerator {
    public:
        enum class Mode {
            Translate = 0,
            Rotate = 1
        };

        static void generateGizmo(Mode mode, std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices) {
            if (mode == Mode::Translate) {
                generateTranslateGizmo(outVertices, outIndices);
            } else if (mode == Mode::Rotate) {
                generateRotateGizmo(outVertices, outIndices);
            }
        }

        static void generateTranslateGizmo(std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices) {
            outVertices.clear();
            outIndices.clear();

            auto add3DArrow = [&](glm::vec3 dir, glm::vec3 color) {
                int segments = 8;
                float radius = 0.04f;
                float shaftLength = 0.75f;
                float tipLength = 0.25f;
                float tipRadius = 0.09f;

                glm::vec3 up = (std::abs(dir.y) < 0.99f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
                glm::vec3 tangent = glm::normalize(glm::cross(dir, up));
                glm::vec3 bitangent = glm::normalize(glm::cross(dir, tangent));

                uint16_t baseIndex = static_cast<uint16_t>(outVertices.size());

                for (int i = 0; i <= segments; ++i) {
                    float theta = (static_cast<float>(i) / segments) * 2.0f * 3.14159265359f;
                    glm::vec3 offset = (tangent * std::cos(theta) + bitangent * std::sin(theta)) * radius;

                    outVertices.push_back({ offset, color, glm::vec3(0), glm::vec2(0) });
                    outVertices.push_back({ (dir * shaftLength) + offset, color, glm::vec3(0), glm::vec2(0) });
                }

                for (int i = 0; i < segments; ++i) {
                    uint16_t b0 = baseIndex + i * 2;
                    uint16_t t0 = baseIndex + i * 2 + 1;
                    uint16_t b1 = baseIndex + (i + 1) * 2;
                    uint16_t t1 = baseIndex + (i + 1) * 2 + 1;

                    outIndices.push_back(b0);
                    outIndices.push_back(b1);
                    outIndices.push_back(t0);

                    outIndices.push_back(t0);
                    outIndices.push_back(b1);
                    outIndices.push_back(t1);
                }

                uint16_t tipBaseIndex = static_cast<uint16_t>(outVertices.size());
                uint16_t tipIndex = tipBaseIndex + segments;

                for (int i = 0; i < segments; ++i) {
                    float theta = (static_cast<float>(i) / segments) * 2.0f * 3.14159265359f;
                    glm::vec3 offset = (tangent * std::cos(theta) + bitangent * std::sin(theta)) * tipRadius;
                    outVertices.push_back({ (dir * shaftLength) + offset, color, glm::vec3(0), glm::vec2(0) });
                }
                outVertices.push_back({ dir * 1.0f, color, glm::vec3(0), glm::vec2(0) });

                for (int i = 0; i < segments; ++i) {
                    uint16_t curr = tipBaseIndex + i;
                    uint16_t next = tipBaseIndex + (i + 1) % segments;

                    outIndices.push_back(tipIndex);
                    outIndices.push_back(curr);
                    outIndices.push_back(next);
                }

                for (int i = 0; i < segments; ++i) {
                    uint16_t st0 = baseIndex + i * 2 + 1;
                    uint16_t st1 = baseIndex + (i + 1) * 2 + 1;
                    uint16_t cb0 = tipBaseIndex + i;
                    uint16_t cb1 = tipBaseIndex + (i + 1) % segments;

                    outIndices.push_back(st0);
                    outIndices.push_back(st1);
                    outIndices.push_back(cb1);

                    outIndices.push_back(st0);
                    outIndices.push_back(cb1);
                    outIndices.push_back(cb0);
                }
            };

            add3DArrow(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.85f, 0.22f, 0.22f));
            add3DArrow(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.22f, 0.75f, 0.22f));
            add3DArrow(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.22f, 0.40f, 0.85f));
        }

        static void generateRotateGizmo(std::vector<Vertex>& outVertices, std::vector<uint16_t>& outIndices) {
            outVertices.clear();
            outIndices.clear();

            auto addRotationRing = [&](glm::vec3 axis, glm::vec3 color) {
                int ringSegments = 36;
                int tubeSegments = 6;
                float ringRadius = 1.0f;
                float tubeRadius = 0.03f;

                glm::vec3 up = (std::abs(axis.y) < 0.99f) ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
                glm::vec3 tangent = glm::normalize(glm::cross(axis, up));
                glm::vec3 bitangent = glm::normalize(glm::cross(axis, tangent));

                uint16_t baseIndex = static_cast<uint16_t>(outVertices.size());

                for (int i = 0; i <= ringSegments; ++i) {
                    float phi = (static_cast<float>(i) / ringSegments) * 2.0f * 3.14159265359f;
                    glm::vec3 ringCenterDir = tangent * std::cos(phi) + bitangent * std::sin(phi);
                    glm::vec3 ringCenter = ringCenterDir * ringRadius;

                    for (int j = 0; j <= tubeSegments; ++j) {
                        float theta = (static_cast<float>(j) / tubeSegments) * 2.0f * 3.14159265359f;
                        glm::vec3 tubeOffset = (ringCenterDir * std::cos(theta) + axis * std::sin(theta)) * tubeRadius;
                        outVertices.push_back({ ringCenter + tubeOffset, color, glm::vec3(0), glm::vec2(0) });
                    }
                }

                for (int i = 0; i < ringSegments; ++i) {
                    for (int j = 0; j < tubeSegments; ++j) {
                        uint16_t row1 = i * (tubeSegments + 1);
                        uint16_t row2 = (i + 1) * (tubeSegments + 1);

                        uint16_t v0 = baseIndex + row1 + j;
                        uint16_t v1 = baseIndex + row1 + j + 1;
                        uint16_t v2 = baseIndex + row2 + j;
                        uint16_t v3 = baseIndex + row2 + j + 1;

                        outIndices.push_back(v0);
                        outIndices.push_back(v2);
                        outIndices.push_back(v1);

                        outIndices.push_back(v1);
                        outIndices.push_back(v2);
                        outIndices.push_back(v3);
                    }
                }
            };

            addRotationRing(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.85f, 0.22f, 0.22f));
            addRotationRing(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.22f, 0.75f, 0.22f));
            addRotationRing(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.22f, 0.40f, 0.85f));
        }
    };

}