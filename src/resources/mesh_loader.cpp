#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "resources/mesh_loader.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <filesystem>
#include <earcut.hpp>

namespace std {
    template<> struct hash<slate::Vertex> {
        size_t operator()(slate::Vertex const& vertex) const {
            size_t h1 = hash<float>()(vertex.pos.x) ^ (hash<float>()(vertex.pos.y) << 1) ^ (hash<float>()(vertex.pos.z) << 2);
            size_t h2 = hash<float>()(vertex.color.x) ^ (hash<float>()(vertex.color.y) << 1) ^ (hash<float>()(vertex.color.z) << 2);
            size_t h3 = hash<float>()(vertex.normal.x) ^ (hash<float>()(vertex.normal.y) << 1) ^ (hash<float>()(vertex.normal.z) << 2);
            size_t h4 = hash<float>()(vertex.texCoord.x) ^ (hash<float>()(vertex.texCoord.y) << 1);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };
}

namespace slate {

    bool MeshLoader::loadOBJ(
        const std::string& filePath,
        std::vector<Vertex>& outVertices,
        std::vector<uint16_t>& outIndices
    ) {
        namespace fs = std::filesystem;
        fs::path objPath(filePath);

        if (!fs::exists(objPath)) {
            std::cerr << "mesh_loader error: file does not exist: " << filePath << std::endl;
            return false;
        }

        fs::path expectedMtlPath = objPath;
        expectedMtlPath.replace_extension(".mtl");

        std::string mtlBaseDir = objPath.parent_path().string();
        if (!mtlBaseDir.empty() && mtlBaseDir.back() != '/' && mtlBaseDir.back() != '\\') {
            mtlBaseDir += "/";
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "mesh_loader error: failed to open file: " << filePath << std::endl;
            return false;
        }

        std::stringstream buffer;
        std::string line;
        bool foundMtllib = false;
        bool mtlNeedsCorrection = false;

        while (std::getline(file, line)) {
            if (line.rfind("mtllib", 0) == 0) {
                foundMtllib = true;
                std::string currentMtlName = line.substr(7);
                fs::path currentMtlPath = fs::path(mtlBaseDir) / currentMtlName;

                // auto correct the mtl
                if (!fs::exists(currentMtlPath) && fs::exists(expectedMtlPath)) {
                    line = "mtllib " + expectedMtlPath.filename().string();
                    mtlNeedsCorrection = true;
                }
            }
            buffer << line << "\n";
        }
        file.close();

        std::string objData = buffer.str();
        if (!foundMtllib && fs::exists(expectedMtlPath)) {
            std::string header = "mtllib " + expectedMtlPath.filename().string() + "\n";
            objData = header + objData;
            mtlNeedsCorrection = true;
        }

        if (mtlNeedsCorrection) {
            std::cout << "[mesh_loader] auto-recovered missing/broken mtllib reference -> "
                      << expectedMtlPath.filename().string() << std::endl;
        }

        std::istringstream objStream(objData);
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        tinyobj::MaterialFileReader matFileReader(mtlBaseDir);

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, &objStream, &matFileReader, false)) {
            std::cerr << "tinyobjloader error: " << err << std::endl;
            return false;
        }

        std::unordered_map<Vertex, uint16_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            size_t index_offset = 0;

            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                size_t fv = shape.mesh.num_face_vertices[f];

                glm::vec3 faceColor(0.8f, 0.8f, 0.8f);
                int materialId = shape.mesh.material_ids[f];

                if (materialId >= 0 && materialId < static_cast<int>(materials.size())) {
                    const auto& mat = materials[materialId];
                    faceColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
                }

                auto getVertex = [&](size_t v_idx) -> Vertex {
                    tinyobj::index_t idx = shape.mesh.indices[index_offset + v_idx];

                    glm::vec3 pos = {
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    };

                    return Vertex(pos, faceColor);
                };

                auto addVertex = [&](const Vertex& v) -> uint16_t {
                    if (uniqueVertices.count(v) == 0) {
                        uniqueVertices[v] = static_cast<uint16_t>(outVertices.size());
                        outVertices.push_back(v);
                    }
                    return uniqueVertices[v];
                };

                if (fv == 3) {
                    outIndices.push_back(addVertex(getVertex(0)));
                    outIndices.push_back(addVertex(getVertex(1)));
                    outIndices.push_back(addVertex(getVertex(2)));
                }
                else if (fv == 4) {
                    uint16_t i0 = addVertex(getVertex(0));
                    uint16_t i1 = addVertex(getVertex(1));
                    uint16_t i2 = addVertex(getVertex(2));
                    uint16_t i3 = addVertex(getVertex(3));

                    outIndices.push_back(i0); outIndices.push_back(i1); outIndices.push_back(i2);
                    outIndices.push_back(i0); outIndices.push_back(i2); outIndices.push_back(i3);
                }
                else if (fv > 4) {
                    std::vector<Vertex> faceVerts;
                    for (size_t v = 0; v < fv; v++) {
                        faceVerts.push_back(getVertex(v));
                    }

                    glm::vec3 faceNormal(0.0f);
                    for (size_t i = 0; i < fv; ++i) {
                        const glm::vec3& vCurrent = faceVerts[i].pos;
                        const glm::vec3& vNext = faceVerts[(i + 1) % fv].pos;
                        faceNormal.x += (vCurrent.y - vNext.y) * (vCurrent.z + vNext.z);
                        faceNormal.y += (vCurrent.z - vNext.z) * (vCurrent.x + vNext.x);
                        faceNormal.z += (vCurrent.x - vNext.x) * (vCurrent.y + vNext.y);
                    }

                    int axisX = 0, axisY = 1;
                    if (std::abs(faceNormal.z) >= std::abs(faceNormal.x) && std::abs(faceNormal.z) >= std::abs(faceNormal.y)) {
                        axisX = 0; axisY = 1;
                    } else if (std::abs(faceNormal.y) >= std::abs(faceNormal.x)) {
                        axisX = 0; axisY = 2;
                    } else {
                        axisX = 1; axisY = 2;
                    }

                    using Point2D = std::array<float, 2>;
                    std::vector<std::vector<Point2D>> polygon(1);

                    for (const auto& v : faceVerts) {
                        polygon[0].push_back({ v.pos[axisX], v.pos[axisY] });
                    }

                    std::vector<uint32_t> earcutIndices = mapbox::earcut<uint32_t>(polygon);

                    for (size_t i = 0; i < earcutIndices.size(); i += 3) {
                        uint32_t i0 = earcutIndices[i];
                        uint32_t i1 = earcutIndices[i + 1];
                        uint32_t i2 = earcutIndices[i + 2];

                        glm::vec3 p0 = faceVerts[i0].pos;
                        glm::vec3 p1 = faceVerts[i1].pos;
                        glm::vec3 p2 = faceVerts[i2].pos;

                        glm::vec3 triNormal = glm::cross(p1 - p0, p2 - p0);

                        if (glm::dot(triNormal, faceNormal) < 0.0f) {
                            std::swap(i1, i2);
                        }

                        outIndices.push_back(addVertex(faceVerts[i0]));
                        outIndices.push_back(addVertex(faceVerts[i1]));
                        outIndices.push_back(addVertex(faceVerts[i2]));
                    }
                }

                index_offset += fv;
            }
        }

        std::cout << "successfully loaded & triangulated obj: " << filePath
                  << " (" << outVertices.size() << " unique vertices, "
                  << outIndices.size() << " indices, "
                  << materials.size() << " materials)" << std::endl;

        return true;
    }

}