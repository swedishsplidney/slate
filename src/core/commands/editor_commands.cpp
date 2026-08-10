#include "editor_commands.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "ui/ui_manager.hpp"
#include <iostream>

namespace slate {
    
    bool ToggleGridCommand::execute(const CommandContext& context) {
        if (context.renderer) {
            context.renderer->toggleGrid();
            std::cout << "[editorcommand] grid toggled, gurrent state: "
                      << (context.renderer->isGridEnabled() ? "ON" : "OFF") << "\n";
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

    bool ToggleGridCommand::undo(const CommandContext& context) {
        if (context.renderer) {
            context.renderer->toggleGrid();
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

    SetGizmoModeCommand::SetGizmoModeCommand(Mode newMode) : m_newMode(newMode) {}

    bool SetGizmoModeCommand::execute(const CommandContext& context) {
        if (context.renderer) {
            auto currentVkMode = context.renderer->getGizmoMode();
            m_previousMode = static_cast<Mode>(currentVkMode);

            context.renderer->setGizmoMode(static_cast<VulkanRenderer::GizmoMode>(m_newMode));
            std::cout << "[editorcommand] gizmo mode changed\n";
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

    bool SetGizmoModeCommand::undo(const CommandContext& context) {
        if (context.renderer) {
            context.renderer->setGizmoMode(static_cast<VulkanRenderer::GizmoMode>(m_previousMode));
            std::cout << "[editorcommand] gizmo mode reverted\n";
        }
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

}