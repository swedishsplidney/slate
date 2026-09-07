#pragma once

#include "command_registry.hpp"
#include <iostream>

namespace slate {

    class TogglePhysicsCommand : public ICommand {
    public:
        TogglePhysicsCommand(bool& physicsRunningFlag)
            : m_physicsRunning(physicsRunningFlag) {}

        bool execute(const CommandContext& context) override {
            (void)context;
            m_physicsRunning = !m_physicsRunning;
            std::cout << "[physics] simulation " << (m_physicsRunning ? "running" : "stopped") << "\n";
            return true;
        }

        bool undo(const CommandContext& context) override {
            return execute(context);
        }

        const std::string& getName() const override {
            static const std::string name = "TogglePhysicsCommand";
            return name;
        }

    private:
        bool& m_physicsRunning;
    };

}