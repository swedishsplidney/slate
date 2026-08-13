#include "translate_commands.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "ui/ui_manager.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace slate {

    TranslateMeshCommand::TranslateMeshCommand(size_t meshIndex, const glm::vec3& translationDelta)
        : m_meshIndex(meshIndex), m_translationDelta(translationDelta) {}

    bool TranslateMeshCommand::execute(const CommandContext& context) {
        if (context.renderer) {
            auto vkRenderer = static_cast<VulkanRenderer*>(context.renderer);
            auto& meshes = vkRenderer->getSceneMeshes();
            if (m_meshIndex < meshes.size() && meshes[m_meshIndex]) {
                glm::mat4 current = meshes[m_meshIndex]->getModelMatrix();
                meshes[m_meshIndex]->setModelMatrix(glm::translate(glm::mat4(1.0f), m_translationDelta) * current);
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
                glm::mat4 current = meshes[m_meshIndex]->getModelMatrix();
                meshes[m_meshIndex]->setModelMatrix(glm::translate(glm::mat4(1.0f), -m_translationDelta) * current);
            }
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

}