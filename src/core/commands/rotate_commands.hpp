#pragma once
#include "command_registry.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

namespace slate {

    class RotateMeshCommand : public ICommand {
    public:
        RotateMeshCommand(size_t meshIndex, const glm::quat& rotationDelta);

        bool execute(const CommandContext& context) override;
        bool undo(const CommandContext& context) override;

        const std::string& getName() const override {
            static const std::string name = "RotateMeshCommand";
            return name;
        }

    private:
        size_t m_meshIndex;
        glm::quat m_rotationDelta;
    };

}