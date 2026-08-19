#define GLM_ENABLE_EXPERIMENTAL
#include "scale_commands.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "ui/ui_manager.hpp"
#include <glm/gtx/matrix_decompose.hpp>

namespace slate {

    SetScaleCommand::SetScaleCommand(size_t meshIndex, const glm::vec3& newScale)
        : m_meshIndex(meshIndex), m_newScale(newScale) {}

    bool SetScaleCommand::execute(const CommandContext& context) {
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

                m_oldScale = scale;

                glm::mat4 newModel = glm::translate(glm::mat4(1.0f), translation) * 
                                     glm::mat4_cast(orientation) * 
                                     glm::scale(glm::mat4(1.0f), m_newScale);
                mesh->setModelMatrix(newModel);
            }
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

    bool SetScaleCommand::undo(const CommandContext& context) {
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

                glm::mat4 oldModel = glm::translate(glm::mat4(1.0f), translation) * 
                                     glm::mat4_cast(orientation) * 
                                     glm::scale(glm::mat4(1.0f), m_oldScale);
                mesh->setModelMatrix(oldModel);
            }
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

}