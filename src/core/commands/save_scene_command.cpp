#include "save_scene_command.hpp"
#include "core/serializer.hpp"

namespace slate {

    SaveSceneCommand::SaveSceneCommand(std::string filepath) : m_filepath(std::move(filepath)) {}

    bool SaveSceneCommand::execute(const CommandContext& context) {
        if (!context.renderer) return false;
        std::string path = m_filepath.empty() ? "scene.slate" : m_filepath;
        return Serializer::saveScene(path, context.renderer);
    }

    bool SaveSceneCommand::undo(const CommandContext& context) {
        // why would you want to undo saving
        return false;
    }

    const std::string& SaveSceneCommand::getName() const {
        static std::string name = "file.save_scene";
        return name;
    }

}