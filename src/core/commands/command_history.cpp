#include "command_history.hpp"
#include <iostream>

namespace slate {

    CommandHistory::CommandHistory(size_t maxHistorySize)
        : m_maxHistorySize(maxHistorySize) {}

    bool CommandHistory::executeAndPush(std::unique_ptr<ICommand> command, const CommandContext& context) {
        if (!command) return false;

        if (command->execute(context)) {
            if (command->isReversible()) {
                if (!m_undoStack.empty() && m_undoStack.back()->mergeWith(command.get())) {
                    return true;
                }

                m_redoStack.clear();

                m_undoStack.push_back(std::move(command));

                if (m_undoStack.size() > m_maxHistorySize) {
                    m_undoStack.pop_front();
                }
            }
            return true;
        }
        return false;
    }

    bool CommandHistory::undo(const CommandContext& context) {
        if (!canUndo()) return false;

        auto command = std::move(m_undoStack.back());
        m_undoStack.pop_back();

        if (command->undo(context)) {
            m_redoStack.push_back(std::move(command));
            return true;
        }
        return false;
    }

    bool CommandHistory::redo(const CommandContext& context) {
        if (!canRedo()) return false;

        auto command = std::move(m_redoStack.back());
        m_redoStack.pop_back();

        if (command->execute(context)) {
            m_undoStack.push_back(std::move(command));
            return true;
        }
        return false;
    }

    bool CommandHistory::canUndo() const { return !m_undoStack.empty(); }
    bool CommandHistory::canRedo() const { return !m_redoStack.empty(); }

    void CommandHistory::clear() {
        m_undoStack.clear();
        m_redoStack.clear();
    }

}