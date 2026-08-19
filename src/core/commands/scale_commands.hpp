#pragma once
#include "core/commands/command.hpp"
#include <glm/glm.hpp>

namespace slate {

    class SetScaleCommand : public ICommand {
    public:
        SetScaleCommand(size_t meshIndex, const glm::vec3& newScale);

        bool execute(const CommandContext& context) override;
        bool undo(const CommandContext& context) override;

        const std::string& getName() const override {
            static const std::string name = "editor.set_scale";
            return name;
        }

    private:
        size_t m_meshIndex;
        glm::vec3 m_newScale;
        glm::vec3 m_oldScale{1.0f};
    };

}