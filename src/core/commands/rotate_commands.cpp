#include "rotate_commands.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "ui/ui_manager.hpp"
#include <iostream>

namespace slate {

    RotateMeshCommand::RotateMeshCommand(size_t meshIndex, const glm::quat& rotationDelta)
        : m_meshIndex(meshIndex), m_rotationDelta(rotationDelta) {}

    bool RotateMeshCommand::execute(const CommandContext& context) {
        if (context.renderer) {
            auto vkRenderer = static_cast<VulkanRenderer*>(context.renderer);
            auto& meshes = vkRenderer->getSceneMeshes();
            if (m_meshIndex < meshes.size() && meshes[m_meshIndex]) {
                meshes[m_meshIndex]->rotate(m_rotationDelta);
            }
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

    bool RotateMeshCommand::undo(const CommandContext& context) {
        if (context.renderer) {
            auto vkRenderer = static_cast<VulkanRenderer*>(context.renderer);
            auto& meshes = vkRenderer->getSceneMeshes();
            if (m_meshIndex < meshes.size() && meshes[m_meshIndex]) {
                meshes[m_meshIndex]->rotate(glm::inverse(m_rotationDelta));
            }
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

}