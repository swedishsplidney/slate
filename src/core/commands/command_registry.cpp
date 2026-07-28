#include "command_registry.hpp"
#include <iostream>

namespace slate {

    void CommandRegistry::registerCommand(const std::string& commandId, CommandFactory factory) {
        m_factories[commandId] = factory;
    }

    bool CommandRegistry::execute(const std::string& commandId, const CommandContext& context, const CommandArgs& args) {
        auto it = m_factories.find(commandId);
        if (it == m_factories.end()) {
            std::cerr << "[commandregistry] unknown command id: " << commandId << std::endl;
            return false;
        }

        std::unique_ptr<ICommand> cmd = it->second(args);
        return m_history.executeAndPush(std::move(cmd), context);
    }

}