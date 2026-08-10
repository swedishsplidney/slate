#pragma once

#include "command.hpp"
#include <string>
#include <cstdint>

namespace slate {

    class ImportMeshCommand : public ICommand {
    public:
        explicit ImportMeshCommand(std::string filePath = "");

        bool execute(const CommandContext& context) override;
        bool undo(const CommandContext& context) override;

        const std::string& getName() const override { return m_name; }
        bool isReversible() const override { return true; }

    private:
        std::string m_name{"Import Mesh"};
        std::string m_filePath;
        uint32_t m_importedMeshId{0};
        size_t m_materialsAddedCount{0};
    };

}