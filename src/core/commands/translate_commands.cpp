#include "translate_commands.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "ui/ui_manager.hpp"
#include <iostream>

namespace slate {

    TranslateMeshCommand::TranslateMeshCommand(size_t meshIndex, const glm::vec3& translationDelta)
        : m_meshIndex(meshIndex), m_translationDelta(translationDelta) {}

    bool TranslateMeshCommand::execute(const CommandContext& context) {
        if (context.renderer) {
            auto vkRenderer = static_cast<VulkanRenderer*>(context.renderer);
            auto& meshes = vkRenderer->getSceneMeshes();
            if (m_meshIndex < meshes.size() && meshes[m_meshIndex]) {
                meshes[m_meshIndex]->translate(m_translationDelta);
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
                meshes[m_meshIndex]->translate(-m_translationDelta);
            }
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

}