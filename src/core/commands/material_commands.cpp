#define GLM_ENABLE_EXPERIMENTAL
#include "material_commands.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "ui/ui_manager.hpp"
#include <iostream>

namespace slate {

    UpdateMaterialCommand::UpdateMaterialCommand(size_t meshIndex, uint32_t materialId, MaterialPropertyType property, const glm::vec4& newValue)
        : m_meshIndex(meshIndex), m_materialId(materialId), m_propertyType(property), m_newVec4(newValue), m_isVector(true) {}

    UpdateMaterialCommand::UpdateMaterialCommand(size_t meshIndex, uint32_t materialId, MaterialPropertyType property, float newValue)
        : m_meshIndex(meshIndex), m_materialId(materialId), m_propertyType(property), m_newFloat(newValue), m_isVector(false) {}

    void UpdateMaterialCommand::applyValue(const CommandContext& context, bool isNew) {
        if (!context.renderer) return;

        auto vkRenderer = static_cast<VulkanRenderer*>(context.renderer);
        auto& materials = vkRenderer->getGlobalMaterials();

        if (m_materialId >= materials.size()) return;

        auto& mat = materials[m_materialId];

        if (isNew) {
            if (m_isVector) {
                switch (m_propertyType) {
                    case MaterialPropertyType::Albedo: m_oldVec4 = mat.gpuData.albedoFactor; break;
                    default: break;
                }
            } else {
                switch (m_propertyType) {
                    case MaterialPropertyType::Roughness: m_oldFloat = mat.gpuData.roughnessFactor; break;
                    case MaterialPropertyType::Metallic: m_oldFloat = mat.gpuData.metallicFactor; break;
                    case MaterialPropertyType::Transmission: m_oldFloat = mat.gpuData.transmissionFactor; break;
                    case MaterialPropertyType::IOR: m_oldFloat = mat.gpuData.ior; break;
                    default: break;
                }
            }
        }

        if (m_isVector) {
            glm::vec4 val = isNew ? m_newVec4 : m_oldVec4;
            switch (m_propertyType) {
                case MaterialPropertyType::Albedo: mat.gpuData.albedoFactor = val; break;
                default: break;
            }
        } else {
            float val = isNew ? m_newFloat : m_oldFloat;
            switch (m_propertyType) {
                case MaterialPropertyType::Roughness: mat.gpuData.roughnessFactor = val; break;
                case MaterialPropertyType::Metallic: mat.gpuData.metallicFactor = val; break;
                case MaterialPropertyType::Transmission: mat.gpuData.transmissionFactor = val; break;
                case MaterialPropertyType::IOR: mat.gpuData.ior = val; break;
                default: break;
            }
        }

        vkRenderer->flushMaterialsToGPU();
    }

    bool UpdateMaterialCommand::execute(const CommandContext& context) {
        applyValue(context, true);
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

    bool UpdateMaterialCommand::undo(const CommandContext& context) {
        applyValue(context, false);
        if (context.uiManager) {
            context.uiManager->markDirty();
        }
        return true;
    }

}