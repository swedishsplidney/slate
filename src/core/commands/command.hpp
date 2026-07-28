#pragma once

#include <string>
#include <memory>
#include <vector>

namespace slate {

    class VulkanRenderer;
    class Scene;
    class UIManager;

    struct CommandContext {
        VulkanRenderer* renderer{nullptr};
        Scene* scene{nullptr};
        UIManager* uiManager{nullptr};
    };

    class ICommand {
    public:
        virtual ~ICommand() = default;

        virtual bool execute(const CommandContext& context) = 0;
        virtual bool undo(const CommandContext& context) = 0;

        virtual const std::string& getName() const = 0;
        virtual bool isReversible() const { return true; }

        virtual bool mergeWith(const ICommand* other) { (void)other; return false; }
    };

}