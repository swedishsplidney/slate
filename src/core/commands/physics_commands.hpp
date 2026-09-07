#pragma once

#include "command_registry.hpp"
#include "resources/mesh_loader.hpp"
#include "json.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

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



    class SetPhysicsPropertyCommand : public ICommand {
    public:
        SetPhysicsPropertyCommand(std::shared_ptr<Mesh> mesh, std::string propertyName, float newValue, float oldValue)
            : m_mesh(mesh), m_propertyName(propertyName), m_newValue(newValue), m_oldValue(oldValue) {}

        bool execute(const CommandContext& context) override {
            (void)context;
            if (!m_mesh) return false;
            applyValue(m_newValue);
            return true;
        }

        bool undo(const CommandContext& context) override {
            (void)context;
            if (!m_mesh) return false;
            applyValue(m_oldValue);
            return true;
        }

        const std::string& getName() const override {
            static const std::string name = "SetPhysicsPropertyCommand";
            return name;
        }

    private:
        void applyValue(float val) {
            if (!m_mesh || m_mesh->getPath().empty()) {
                std::cout << "[physics error] mesh path is empty, cannot save property.\n";
                return;
            }

            fs::path jsonPath(m_mesh->getPath());
            jsonPath.replace_extension(".json");

            nlohmann::json j;

            if (fs::exists(jsonPath)) {
                std::ifstream inFile(jsonPath);
                if (inFile.is_open()) {
                    try {
                        inFile >> j;
                    } catch (...) {
                        j = nlohmann::json::object();
                    }
                }
            }

            if (!j.contains("physics") || !j["physics"].is_object()) {
                j["physics"] = nlohmann::json::object();
            }

            j["physics"][m_propertyName] = val;

            std::string fileContent = j.dump(4);

            std::ofstream outFile(jsonPath);
            if (outFile.is_open()) {
                outFile << fileContent;
            } else {
                std::cout << "[physics error] failed to open JSON sidecar for writing: " << jsonPath.string() << "\n";
                return;
            }

            m_mesh->setPhysicsProperty(m_propertyName, val);

            std::cout << "[physics] Setting " << m_propertyName << " to " << val
                      << " for mesh: " << m_mesh->getName() << "\n";
        }

        std::shared_ptr<Mesh> m_mesh;
        std::string m_propertyName;
        float m_newValue;
        float m_oldValue;
    };

}