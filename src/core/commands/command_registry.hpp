#pragma once

#include "command.hpp"
#include "command_history.hpp"
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>

namespace slate {

    class CommandRegistry {
    public:
        using CommandArgs = std::vector<std::string>;
        using CommandFactory = std::function<std::unique_ptr<ICommand>(const CommandArgs&)>;

        CommandRegistry() = default;
        ~CommandRegistry() = default;

        void registerCommand(const std::string& commandId, CommandFactory factory);

        bool execute(const std::string& commandId, const CommandContext& context, const CommandArgs& args = {});

        CommandHistory& getHistory() { return m_history; }

    private:
        std::unordered_map<std::string, CommandFactory> m_factories;
        CommandHistory m_history{100};
    };

}