#pragma once

#include "command.hpp"
#include <vector>
#include <memory>
#include <deque>

namespace slate {

    class CommandHistory {
    public:
        explicit CommandHistory(size_t maxHistorySize = 100);
        ~CommandHistory() = default;

        bool executeAndPush(std::unique_ptr<ICommand> command, const CommandContext& context);

        bool undo(const CommandContext& context);
        bool redo(const CommandContext& context);

        bool canUndo() const;
        bool canRedo() const;

        void clear();

        const std::deque<std::unique_ptr<ICommand>>& getUndoStack() const { return m_undoStack; }

    private:
        size_t m_maxHistorySize{100};
        std::deque<std::unique_ptr<ICommand>> m_undoStack;
        std::vector<std::unique_ptr<ICommand>> m_redoStack;
    };

}