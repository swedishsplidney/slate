#pragma once

#include "core/commands/command.hpp"
#include <glm/glm.hpp>

namespace slate {

    enum class MaterialPropertyType {
        Albedo,
        Roughness,
        Metallic,
        Transmission,
        IOR
    };

    class UpdateMaterialCommand : public ICommand {
    public:
        UpdateMaterialCommand(size_t meshIndex, uint32_t materialId, MaterialPropertyType property, const glm::vec4& newValue);

        UpdateMaterialCommand(size_t meshIndex, uint32_t materialId, MaterialPropertyType property, float newValue);

        bool execute(const CommandContext& context) override;
        bool undo(const CommandContext& context) override;

        const std::string& getName() const override {
            static const std::string name = "material.update";
            return name;
        }

    private:
        void applyValue(const CommandContext& context, bool isNew);

        size_t m_meshIndex;
        uint32_t m_materialId;
        MaterialPropertyType m_propertyType;

        glm::vec4 m_newVec4{0.0f};
        glm::vec4 m_oldVec4{0.0f};
        float m_newFloat{0.0f};
        float m_oldFloat{0.0f};
        bool m_isVector{false};
    };

}