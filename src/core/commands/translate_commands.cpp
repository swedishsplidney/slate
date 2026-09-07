#define GLM_ENABLE_EXPERIMENTAL
#include "translate_commands.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "ui/ui_manager.hpp"
#include "physics/physics_engine.hpp"
#include <iostream>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace slate {

    TranslateMeshCommand::TranslateMeshCommand(size_t meshIndex, const glm::vec3& translationDelta)
        : m_meshIndex(meshIndex), m_translationDelta(translationDelta) {}

    bool TranslateMeshCommand::execute(const CommandContext& context) {
        if (context.renderer) {
            auto vkRenderer = static_cast<VulkanRenderer*>(context.renderer);
            auto& meshes = vkRenderer->getSceneMeshes();
            if (m_meshIndex < meshes.size() && meshes[m_meshIndex]) {
                auto& mesh = meshes[m_meshIndex];

                glm::mat4 modelMat = mesh->getModelMatrix();
                glm::vec3 scale, translation;
                glm::quat orientation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(modelMat, scale, orientation, translation, skew, perspective);

                glm::vec3 localDelta = glm::inverse(orientation) * m_translationDelta;
                mesh->translate(localDelta);

                // jolt sync
                if (context.physicsEngine && !mesh->getBodyID().IsInvalid()) {
                    auto& bodyInterface = context.physicsEngine->getPhysicsSystem().GetBodyInterface();
                    JPH::BodyID bodyId = mesh->getBodyID();

                    glm::mat4 newModelMat = mesh->getModelMatrix();
                    glm::decompose(newModelMat, scale, orientation, translation, skew, perspective);

                    JPH::RVec3 joltPos(translation.x, translation.y, translation.z);
                    JPH::Quat joltRot(orientation.x, orientation.y, orientation.z, orientation.w);

                    bodyInterface.SetPositionAndRotation(bodyId, joltPos, joltRot, JPH::EActivation::Activate);
                }
            }
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

    bool TranslateMeshCommand::undo(const CommandContext& context) {
        if (context.renderer) {
            auto vkRenderer = static_cast<VulkanRenderer*>(context.renderer);
            auto& meshes = vkRenderer->getSceneMeshes();
            if (m_meshIndex < meshes.size() && meshes[m_meshIndex]) {
                auto& mesh = meshes[m_meshIndex];

                glm::mat4 modelMat = mesh->getModelMatrix();
                glm::vec3 scale, translation;
                glm::quat orientation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(modelMat, scale, orientation, translation, skew, perspective);

                glm::vec3 localDelta = glm::inverse(orientation) * m_translationDelta;
                mesh->translate(-localDelta);

                // jolt sync
                if (context.physicsEngine && !mesh->getBodyID().IsInvalid()) {
                    auto& bodyInterface = context.physicsEngine->getPhysicsSystem().GetBodyInterface();
                    JPH::BodyID bodyId = mesh->getBodyID();

                    glm::mat4 newModelMat = mesh->getModelMatrix();
                    glm::decompose(newModelMat, scale, orientation, translation, skew, perspective);

                    JPH::RVec3 joltPos(translation.x, translation.y, translation.z);
                    JPH::Quat joltRot(orientation.x, orientation.y, orientation.z, orientation.w);

                    bodyInterface.SetPositionAndRotation(bodyId, joltPos, joltRot, JPH::EActivation::Activate);
                }
            }
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

}