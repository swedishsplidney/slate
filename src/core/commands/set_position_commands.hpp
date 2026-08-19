#pragma once
#include "core/commands/command.hpp"
#include <glm/glm.hpp>

namespace slate {

    class SetPositionCommand : public ICommand {
    public:
        SetPositionCommand(size_t meshIndex, const glm::vec3& newPosition);

        bool execute(const CommandContext& context) override;
        bool undo(const CommandContext& context) override;

        const std::string& getName() const override {
            static const std::string name = "editor.set_position";
            return name;
        }

    private:
        size_t m_meshIndex;
        glm::vec3 m_newPosition;
        glm::vec3 m_oldPosition{0.0f};
    };

}