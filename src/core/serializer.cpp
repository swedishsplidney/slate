#define GLM_ENABLE_EXPERIMENTAL
#include "serializer.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace slate {

    bool Serializer::saveScene(const std::string& filepath, VulkanRenderer* renderer) {
        if (!renderer) {
            std::cout << "[serializer error] renderer is null, cannot save scene.\n";
            return false;
        }

        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cout << "[serializer error] failed to open file for writing: " << filepath << "\n";
            return false;
        }

        // file header
        file << "[scene]\n";
        file << "name = \"SlateScene\"\n";
        file << "version = \"1.0\"\n\n";

        const auto& sceneMeshes = renderer->getSceneMeshes();
        const auto& globalMaterials = renderer->getGlobalMaterials();

        for (size_t i = 0; i < sceneMeshes.size(); ++i) {
            auto& mesh = sceneMeshes[i];
            if (!mesh) continue;

            file << "[[entity]]\n";
            file << "index = " << i << "\n";
            file << "name = \"" << mesh->getName() << "\"\n";

            file << "mesh_path = \"" << mesh->getPath() << "\"\n";

            // decomp model matrix
            glm::mat4 modelMat = mesh->getModelMatrix();
            glm::vec3 scale(glm::length(modelMat[0]), glm::length(modelMat[1]), glm::length(modelMat[2]));
            glm::mat3 rotMat(
                modelMat[0] / (scale.x != 0.0f ? scale.x : 1.0f),
                modelMat[1] / (scale.y != 0.0f ? scale.y : 1.0f),
                modelMat[2] / (scale.z != 0.0f ? scale.z : 1.0f)
            );
            glm::vec3 rotation = glm::degrees(glm::eulerAngles(glm::quat_cast(rotMat)));
            glm::vec3 position = glm::vec3(modelMat[3]);

            file << "position = [" << position.x << ", " << position.y << ", " << position.z << "]\n";
            file << "rotation = [" << rotation.x << ", " << rotation.y << ", " << rotation.z << "]\n";
            file << "scale = [" << scale.x << ", " << scale.y << ", " << scale.z << "]\n";

            // materials
            uint32_t materialId = mesh->getMaterialId();
            if (materialId < globalMaterials.size()) {
                const auto& mat = globalMaterials[materialId];
                file << "[entity.material]\n";
                file << "albedo = [" << mat.gpuData.albedoFactor.r << ", "
                                     << mat.gpuData.albedoFactor.g << ", "
                                     << mat.gpuData.albedoFactor.b << ", "
                                     << mat.gpuData.albedoFactor.a << "]\n";
                file << "roughness = " << mat.gpuData.roughnessFactor << "\n";
                file << "metallic = " << mat.gpuData.metallicFactor << "\n";
                file << "ior = " << mat.gpuData.ior << "\n";
                file << "transmission = " << mat.gpuData.transmissionFactor << "\n";
            }

            file << "\n";
        }

        file.close();
        std::cout << "[serializer] scene successfully saved to " << filepath << "\n";
        return true;
    }

    static glm::vec3 parseVec3(const std::string& line) {
        size_t start = line.find('[');
        size_t end = line.find(']');
        if (start == std::string::npos || end == std::string::npos) return glm::vec3(0.0f);

        std::stringstream ss(line.substr(start + 1, end - start - 1));
        std::string val;
        float x = 0, y = 0, z = 0;
        if (std::getline(ss, val, ',')) x = std::stof(val);
        if (std::getline(ss, val, ',')) y = std::stof(val);
        if (std::getline(ss, val)) z = std::stof(val);
        return glm::vec3(x, y, z);
    }

    static glm::vec4 parseVec4(const std::string& line) {
        size_t start = line.find('[');
        size_t end = line.find(']');
        if (start == std::string::npos || end == std::string::npos) return glm::vec4(1.0f);

        std::stringstream ss(line.substr(start + 1, end - start - 1));
        std::string val;
        float x = 1, y = 1, z = 1, w = 1;
        if (std::getline(ss, val, ',')) x = std::stof(val);
        if (std::getline(ss, val, ',')) y = std::stof(val);
        if (std::getline(ss, val, ',')) z = std::stof(val);
        if (std::getline(ss, val)) w = std::stof(val);
        return glm::vec4(x, y, z, w);
    }

    bool Serializer::loadScene(const std::string& filepath, VulkanRenderer* renderer) {
        if (!renderer) {
            std::cout << "[serializer error] renderer is null, cannot load scene.\n";
            return false;
        }

        renderer->clearScene();

        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cout << "[serializer error] failed to open file for reading: " << filepath << "\n";
            return false;
        }

        std::string line;
        std::string currentMeshPath = "";
        std::string currentEntityName = "";
        glm::vec3 pos(0.0f), rot(0.0f), scale(1.0f);
        glm::vec4 albedo(1.0f);
        float roughness = 0.5f, metallic = 0.0f, ior = 1.5f, transmission = 0.0f;
        bool hasEntity = false;

        auto spawnEntity = [&]() {
            if (currentMeshPath.empty()) return;

            std::vector<Vertex> vertices;
            std::vector<uint16_t> indices;
            auto& globalMaterials = renderer->getGlobalMaterials();
            size_t matsBefore = globalMaterials.size();

            bool loaded = false;
            if (currentMeshPath.find(".obj") != std::string::npos) {
                loaded = MeshLoader::loadOBJ(currentMeshPath, vertices, indices, globalMaterials);
            }

            if (loaded && !vertices.empty()) {
                renderer->flushMaterialsToGPU();

                uint32_t matId = 0;
                if (globalMaterials.size() > matsBefore) {
                    matId = globalMaterials[matsBefore].materialId;
                    globalMaterials[matsBefore].gpuData.albedoFactor = albedo;
                    globalMaterials[matsBefore].gpuData.roughnessFactor = roughness;
                    globalMaterials[matsBefore].gpuData.metallicFactor = metallic;
                    globalMaterials[matsBefore].gpuData.ior = ior;
                    globalMaterials[matsBefore].gpuData.transmissionFactor = transmission;
                }

                bool transparent = (transmission > 0.0f || albedo.a < 1.0f);
                auto newMesh = std::make_unique<Mesh>(
                    renderer->getDevice(),
                    renderer->getPhysicalDevice(),
                    vertices,
                    indices,
                    matId,
                    transparent
                );

                newMesh->setName(currentEntityName.empty() ? "LoadedMesh" : currentEntityName);
                newMesh->setPath(currentMeshPath);

                glm::mat4 model(1.0f);
                model = glm::translate(model, pos);
                glm::quat q(glm::radians(rot));
                model = model * glm::mat4_cast(q);
                model = glm::scale(model, scale);
                newMesh->setModelMatrix(model);

                renderer->addMeshToScene(std::move(newMesh));
            }
        };

        while (std::getline(file, line)) {
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.rfind("[[entity]]", 0) == 0) {
                if (hasEntity) {
                    spawnEntity();
                }
                hasEntity = true;
                currentMeshPath = "";
                currentEntityName = "";
                pos = glm::vec3(0.0f);
                rot = glm::vec3(0.0f);
                scale = glm::vec3(1.0f);
                albedo = glm::vec4(1.0f);
                roughness = 0.5f;
                metallic = 0.0f;
                ior = 1.5f;
                transmission = 0.0f;
            } else if (line.rfind("name = ", 0) == 0 && hasEntity && currentEntityName.empty()) {
                size_t start = line.find('"');
                size_t end = line.rfind('"');
                if (start != std::string::npos && end != std::string::npos && end > start) {
                    currentEntityName = line.substr(start + 1, end - start - 1);
                }
            } else if (line.rfind("mesh_path = ", 0) == 0) {
                size_t start = line.find('"');
                size_t end = line.rfind('"');
                if (start != std::string::npos && end != std::string::npos && end > start) {
                    currentMeshPath = line.substr(start + 1, end - start - 1);
                }
            } else if (line.rfind("position = ", 0) == 0) {
                pos = parseVec3(line);
            } else if (line.rfind("rotation = ", 0) == 0) {
                rot = parseVec3(line);
            } else if (line.rfind("scale = ", 0) == 0) {
                scale = parseVec3(line);
            } else if (line.rfind("albedo = ", 0) == 0) {
                albedo = parseVec4(line);
            } else if (line.rfind("roughness = ", 0) == 0) {
                roughness = std::stof(line.substr(line.find('=') + 1));
            } else if (line.rfind("metallic = ", 0) == 0) {
                metallic = std::stof(line.substr(line.find('=') + 1));
            } else if (line.rfind("ior = ", 0) == 0) {
                ior = std::stof(line.substr(line.find('=') + 1));
            } else if (line.rfind("transmission = ", 0) == 0) {
                transmission = std::stof(line.substr(line.find('=') + 1));
            }
        }

        if (hasEntity) {
            spawnEntity();
        }

        file.close();
        renderer->flushMaterialsToGPU();
        std::cout << "[serializer] scene successfully loaded from " << filepath << "\n";
        return true;
    }

}