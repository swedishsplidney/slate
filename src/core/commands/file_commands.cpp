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
        std::vector<Material> loadedMaterials;

        bool success = false;

        // Auto-detect format (easy to expand for .gltf / .stl later)
        if (ext == ".obj") {
            success = MeshLoader::loadOBJ(m_filePath, loadedVertices, loadedIndices, loadedMaterials);
        } else {
            std::cerr << "[importmeshcommand] unsupported format: " << ext << std::endl;
            return false;
        }

        if (!success || loadedVertices.empty()) {
            std::cerr << "[importmeshcommand] mesh loading failed or produced empty geometry.\n";
            return false;
        }

        // 2. Upload mesh and materials to Vulkan renderer
        if (context.renderer) {
            if (!loadedMaterials.empty()) {
                context.renderer->updateMaterials(loadedMaterials);
            }

            auto newMesh = std::make_unique<Mesh>(
                context.renderer->getDevice(),
                context.renderer->getPhysicalDevice(),
                loadedVertices,
                loadedIndices
            );

            m_importedMeshId = context.renderer->addMeshToScene(std::move(newMesh));
        }

        // 3. Mark UI dirty so the frame refreshes smoothly
        if (context.uiManager) {
            context.uiManager->markDirty();
        }

        std::cout << "[importmeshcommand] successfully imported " << path.filename().string() << std::endl;
        return true;
    }

    bool ImportMeshCommand::undo(const CommandContext& context) {
        if (context.renderer && m_importedMeshId != 0) {
            if (context.uiManager) {
                context.uiManager->markDirty();
            }

            std::cout << "[ImportMeshCommand] Undid import of mesh ID: " << m_importedMeshId << std::endl;
            return true;
        }
        return false;
    }

}