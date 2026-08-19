#pragma once

#include "core/commands/command.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace slate {

    class SetRotationCommand : public ICommand {
    public:
        SetRotationCommand(size_t meshIndex, const glm::quat& newRotation);

        bool execute(const CommandContext& context) override;
        bool undo(const CommandContext& context) override;

        const std::string& getName() const override {
            static const std::string name = "editor.set_rotation";
            return name;
        }

    private:
        size_t m_meshIndex;
        glm::quat m_newRotation;
        glm::quat m_oldRotation{1.0f, 0.0f, 0.0f, 0.0f};
    };

}