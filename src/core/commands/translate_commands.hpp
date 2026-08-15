#pragma once
#include "command_registry.hpp"
#include <glm/glm.hpp>
#include <string>

namespace slate {

    class TranslateMeshCommand : public ICommand {
    public:
        TranslateMeshCommand(size_t meshIndex, const glm::vec3& translationDelta);

        bool execute(const CommandContext& context) override;
        bool undo(const CommandContext& context) override;

        const std::string& getName() const override {
            static const std::string name = "TranslateMeshCommand";
            return name;
        }

    private:
        size_t m_meshIndex;
        glm::vec3 m_translationDelta;
    };

}