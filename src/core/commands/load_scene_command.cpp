#include "load_scene_command.hpp"
#include "core/serializer.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "ui/ui_manager.hpp"
#include "portable-file-dialogs.h"
#include <iostream>
#include <filesystem>

namespace slate {

    LoadSceneCommand::LoadSceneCommand(std::string filepath) : m_filepath(std::move(filepath)) {}

    bool LoadSceneCommand::execute(const CommandContext& context) {
        if (m_filepath.empty()) {
            auto openDialog = pfd::open_file(
                "Select Slate Scene to Load",
                ".",
                { "Slate Scene Files (.slate)", "*.slate",
                  "All Files", "*" },
                pfd::opt::none
            );

            auto result = openDialog.result();
            if (result.empty()) {
                std::cout << "[loadscenecommand] load cancelled by user.\n";
                return false;
            }
            m_filepath = result[0];
        }

        if (!context.renderer) {
            std::cerr << "[loadscenecommand] renderer is null.\n";
            return false;
        }

        bool success = Serializer::loadScene(m_filepath, context.renderer);

        if (success && context.uiManager) {
            context.uiManager->markDirty();
        }

        std::cout << "[loadscenecommand] successfully processed scene: " << m_filepath << "\n";
        return success;
    }

    bool LoadSceneCommand::undo(const CommandContext& context) {
        // why would you want to undo loading
        return false;
    }

    const std::string& LoadSceneCommand::getName() const {
        static std::string name = "file.load_scene";
        return name;
    }

}