#pragma once

#include "command.hpp"
#include <string>

namespace slate {

    class ToggleGridCommand : public ICommand {
    public:
        ToggleGridCommand() = default;
        bool execute(const CommandContext& context) override;
        bool undo(const CommandContext& context) override;
        const std::string& getName() const override { static std::string name = "Toggle Grid"; return name; }
    };

    class SetGizmoModeCommand : public ICommand {
    public:
        enum class Mode { Translate, Rotate, Scale };

        explicit SetGizmoModeCommand(Mode newMode);
        bool execute(const CommandContext& context) override;
        bool undo(const CommandContext& context) override;
        const std::string& getName() const override { static std::string name = "Set Gizmo Mode"; return name; }

    private:
        Mode m_newMode;
        Mode m_previousMode{Mode::Translate};
    };

}