#include "file_commands.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "resources/mesh_loader.hpp"
#include "ui/ui_manager.hpp"
#include "portable-file-dialogs.h"

#include <iostream>
#include <filesystem>

namespace slate {

    ImportMeshCommand::ImportMeshCommand(std::string filePath)
        : m_filePath(std::move(filePath)) {}

    bool ImportMeshCommand::execute(const CommandContext& context) {
        if (m_filePath.empty()) {
            auto openDialog = pfd::open_file(
                "Select 3D Mesh to Import",
                ".",
                { "Supported 3D Models (.obj)", "*.obj",
                  "All Files", "*" },
                pfd::opt::none
            );

            auto result = openDialog.result();
            if (result.empty()) {
                std::cout << "[importmeshcommand] import cancelled by user.\n";
                return false;
            }
            m_filePath = result[0];
        }

        std::filesystem::path path(m_filePath);
        std::string ext = path.extension().string();

        std::vector<Vertex> loadedVertices;
        std::vector<uint16_t> loadedIndices;

        bool success = false;

        if (context.renderer) {
            auto& globalMaterials = context.renderer->getGlobalMaterials();

            size_t materialsBeforeLoad = globalMaterials.size();

            if (ext == ".obj") {
                success = MeshLoader::loadOBJ(m_filePath, loadedVertices, loadedIndices, globalMaterials);
            } else {
                std::cerr << "[importmeshcommand] unsupported format: " << ext << std::endl;
                return false;
            }

            if (!success || loadedVertices.empty()) {
                std::cerr << "[importmeshcommand] mesh loading failed or produced empty geometry.\n";
                return false;
            }

            m_materialsAddedCount = globalMaterials.size() - materialsBeforeLoad;

            uint32_t primaryMaterialId = 0;
            bool isTransparent = false;

            if (globalMaterials.size() > materialsBeforeLoad) {
                const auto& primaryMat = globalMaterials[materialsBeforeLoad];
                primaryMaterialId = primaryMat.materialId;

                if (primaryMat.gpuData.transmissionFactor > 0.0f ||
                    primaryMat.gpuData.albedoFactor.a < 1.0f) {
                    isTransparent = true;
                }
            }

            context.renderer->flushMaterialsToGPU();

            auto newMesh = std::make_unique<Mesh>(
                context.renderer->getDevice(),
                context.renderer->getPhysicalDevice(),
                loadedVertices,
                loadedIndices,
                primaryMaterialId,
                isTransparent
            );

            newMesh->setPath(m_filePath);

            m_importedMeshId = context.renderer->addMeshToScene(std::move(newMesh));
        }

        if (context.uiManager) {
            context.uiManager->markDirty();
        }

        std::cout << "[importmeshcommand] successfully imported " << path.filename().string() << "\n";
        return true;
    }

    bool ImportMeshCommand::undo(const CommandContext& context) {
        if (context.renderer && m_importedMeshId != 0) {
            context.renderer->removeLastMeshFromScene();

            if (m_materialsAddedCount > 0) {
                context.renderer->popGlobalMaterials(m_materialsAddedCount);
                context.renderer->flushMaterialsToGPU();
            }

            if (context.uiManager) {
                context.uiManager->markDirty();
            }

            std::cout << "[importmeshcommand] undid import of mesh ID: " << m_importedMeshId << std::endl;
            return true;
        }
        return false;
    }

}