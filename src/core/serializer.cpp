#define GLM_ENABLE_EXPERIMENTAL
#include "serializer.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include <fstream>
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

            file << "mesh_path = \"models/model.obj\"\n";

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

    bool Serializer::loadScene(const std::string& filepath, VulkanRenderer* renderer) {
        std::cout << "[serializer] load scene requested for: " << filepath << "\n";
        return true;
    }

}