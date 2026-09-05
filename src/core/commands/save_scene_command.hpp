#pragma once
#include "command.hpp"
#include <string>

namespace slate {
    class SaveSceneCommand : public ICommand {
    public:
        explicit SaveSceneCommand(std::string filepath);
        bool execute(const CommandContext& context) override;
        bool undo(const CommandContext& context) override;
        const std::string& getName() const override;

    private:
        std::string m_filepath;
    };
}