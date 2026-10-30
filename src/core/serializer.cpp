#define GLM_ENABLE_EXPERIMENTAL
#include "serializer.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "resources/mesh_loader.hpp"
#include "physics/physics_engine.hpp"
#include "physics/collider_generator.hpp"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <glm/gtc/matrix_transform.hpp>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>

namespace slate {

    bool Serializer::saveScene(const std::string& filepath, VulkanRenderer* renderer) {
        if (!renderer) {
            std::cout << "[serializer error] renderer is null, cannot save scene.\n";
            return false;
        }

        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cout << "[serializer error] failed to open file for writing: " << filepath << "\n";
            return false;
        }

        // file header
        file << "[scene]\n";
        file << "name = \"SlateScene\"\n";
        file << "version = \"1.0\"\n\n";

        const auto& sceneMeshes = renderer->getSceneMeshes();
        const auto& globalMaterials = renderer->getGlobalMaterials();

        for (size_t i = 0; i < sceneMeshes.size(); ++i) {
            auto& mesh = sceneMeshes[i];
            if (!mesh) continue;

            file << "[[entity]]\n";
            file << "index = " << i << "\n";
            std::string meshName = mesh->getName();
            if (meshName.empty()) meshName = std::filesystem::path(mesh->getPath()).stem().string();
            file << "name = \"" << meshName << "\"\n";

            file << "mesh_path = \"" << mesh->getPath() << "\"\n";

            // decomp model matrix
            glm::mat4 modelMat = mesh->getModelMatrix();
            glm::vec3 scale(glm::length(modelMat[0]), glm::length(modelMat[1]), glm::length(modelMat[2]));
            glm::mat3 rotMat(
                modelMat[0] / (scale.x != 0.0f ? scale.x : 1.0f),
                modelMat[1] / (scale.y != 0.0f ? scale.y : 1.0f),
                modelMat[2] / (scale.z != 0.0f ? scale.z : 1.0f)
            );
            glm::vec3 rotation = glm::degrees(glm::eulerAngles(glm::quat_cast(rotMat)));
            glm::vec3 position = glm::vec3(modelMat[3]);

            file << "position = [" << position.x << ", " << position.y << ", " << position.z << "]\n";
            file << "rotation = [" << rotation.x << ", " << rotation.y << ", " << rotation.z << "]\n";
            file << "scale = [" << scale.x << ", " << scale.y << ", " << scale.z << "]\n";

            // materials
            uint32_t materialId = mesh->getMaterialId();
            if (materialId < globalMaterials.size()) {
                const auto& mat = globalMaterials[materialId];
                file << "[entity.material]\n";
                file << "albedo = [" << mat.gpuData.albedoFactor.r << ", "
                                     << mat.gpuData.albedoFactor.g << ", "
                                     << mat.gpuData.albedoFactor.b << ", "
                                     << mat.gpuData.albedoFactor.a << "]\n";
                file << "roughness = " << mat.gpuData.roughnessFactor << "\n";
                file << "metallic = " << mat.gpuData.metallicFactor << "\n";
                file << "ior = " << mat.gpuData.ior << "\n";
                file << "transmission = " << mat.gpuData.transmissionFactor << "\n";
            }

            // physics
            if (mesh->getBodyID().IsInvalid() == false && mesh->getPhysicsSystem() != nullptr) {
                auto& bodyInterface = mesh->getPhysicsSystem()->GetBodyInterface();
                JPH::BodyID bodyId = mesh->getBodyID();

                file << "[entity.physics]\n";
                file << "motion_type = " << static_cast<int>(bodyInterface.GetMotionType(bodyId)) << "\n";
                file << "friction = " << bodyInterface.GetFriction(bodyId) << "\n";
                file << "restitution = " << bodyInterface.GetRestitution(bodyId) << "\n";
                file << "gravity_factor = " << bodyInterface.GetGravityFactor(bodyId) << "\n";

                float mass = 1.0f;
                JPH::BodyLockRead lock(mesh->getPhysicsSystem()->GetBodyLockInterface(), bodyId);
                if (lock.Succeeded()) {
                    const JPH::Body& body = lock.GetBody();
                    if (body.IsDynamic() && body.GetMotionProperties() != nullptr) {
                        float invMass = body.GetMotionProperties()->GetInverseMass();
                        if (invMass > 0.0f) mass = 1.0f / invMass;
                    }
                }
                file << "mass = " << mass << "\n";
            }

            file << "\n";
        }

        file.close();
        std::cout << "[serializer] scene successfully saved to " << filepath << "\n";
        return true;
    }

    static glm::vec3 parseVec3(const std::string& line) {
        size_t start = line.find('[');
        size_t end = line.find(']');
        if (start == std::string::npos || end == std::string::npos) return glm::vec3(0.0f);

        std::stringstream ss(line.substr(start + 1, end - start - 1));
        std::string val;
        float x = 0, y = 0, z = 0;
        if (std::getline(ss, val, ',')) x = std::stof(val);
        if (std::getline(ss, val, ',')) y = std::stof(val);
        if (std::getline(ss, val)) z = std::stof(val);
        return glm::vec3(x, y, z);
    }

    static glm::vec4 parseVec4(const std::string& line) {
        size_t start = line.find('[');
        size_t end = line.find(']');
        if (start == std::string::npos || end == std::string::npos) return glm::vec4(1.0f);

        std::stringstream ss(line.substr(start + 1, end - start - 1));
        std::string val;
        float x = 1, y = 1, z = 1, w = 1;
        if (std::getline(ss, val, ',')) x = std::stof(val);
        if (std::getline(ss, val, ',')) y = std::stof(val);
        if (std::getline(ss, val, ',')) z = std::stof(val);
        if (std::getline(ss, val)) w = std::stof(val);
        return glm::vec4(x, y, z, w);
    }

    bool Serializer::loadScene(const std::string& filepath, VulkanRenderer* renderer, PhysicsEngine* physics) {
        if (!renderer) {
            std::cout << "[serializer error] renderer is null, cannot load scene.\n";
            return false;
        }

        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cout << "[serializer error] failed to open file for reading: " << filepath << "\n";
            return false;
        }

        for (auto& mesh : renderer->getSceneMeshes()) {
            if (!mesh) continue;
            JPH::BodyID id = mesh->getBodyID();
            JPH::PhysicsSystem* ps = mesh->getPhysicsSystem();
            if (id.IsInvalid() || !ps) continue;

            auto& bi = ps->GetBodyInterface();
            if (bi.IsAdded(id)) bi.RemoveBody(id);
            bi.DestroyBody(id);
            mesh->setBodyID(JPH::BodyID());
        }

        renderer->clearScene();

        std::string line;
        std::string currentMeshPath = "";
        std::string currentEntityName = "";
        glm::vec3 pos(0.0f), rot(0.0f), scale(1.0f);
        glm::vec4 albedo(1.0f);
        float roughness = 0.5f, metallic = 0.0f, ior = 1.5f, transmission = 0.0f;
        bool hasEntity = false;
        float friction = 0.5f, restitution = 0.2f, gravityFactor = 1.0f, mass = 1.0f;
        int motionTypeInt = 2;
        bool hasPhysics = false;

        auto spawnEntity = [&]() {
            if (currentMeshPath.empty()) return;

            std::vector<Vertex> vertices;
            std::vector<UIIndex> indices;
            auto& globalMaterials = renderer->getGlobalMaterials();
            size_t matsBefore = globalMaterials.size();

            bool loaded = false;
            if (currentMeshPath.find(".obj") != std::string::npos) {
                loaded = MeshLoader::loadOBJ(currentMeshPath, vertices, indices, globalMaterials);
            }
            if (!loaded || vertices.empty()) {
                std::cerr << "[serializer error] failed to load mesh: " << currentMeshPath << "\n";
                return;
            }

            uint32_t matId = 0;
            if (globalMaterials.size() > matsBefore) {
                auto& m = globalMaterials[matsBefore];
                matId = m.materialId;
                m.gpuData.albedoFactor = albedo;
                m.gpuData.roughnessFactor = roughness;
                m.gpuData.metallicFactor = metallic;
                m.gpuData.ior = ior;
                m.gpuData.transmissionFactor = transmission;
            }

            renderer->flushMaterialsToGPU();
            for (size_t i = matsBefore; i < globalMaterials.size(); ++i) {
                renderer->createMaterialDescriptorSets(globalMaterials[i]);
            }

            JPH::ShapeRefC shape;
            if (hasPhysics && physics) {
                auto shapeResult = ColliderGenerator::createOptimalShape(vertices);
                if (!shapeResult.HasError()) shape = shapeResult.Get();
            }

            bool transparent = (transmission > 0.0f || albedo.a < 1.0f);
            auto newMesh = std::make_unique<Mesh>(
                renderer->getDevice(), renderer->getPhysicalDevice(),
                vertices, indices, matId, transparent);

            std::string meshName = currentEntityName;
            if (meshName.empty()) meshName = std::filesystem::path(currentMeshPath).stem().string();
            if (meshName.empty()) meshName = "LoadedMesh";
            newMesh->setName(meshName);
            newMesh->setPath(currentMeshPath);

            glm::mat4 model(1.0f);
            model = glm::translate(model, pos);
            glm::quat q(glm::radians(rot));
            model = model * glm::mat4_cast(q);
            model = glm::scale(model, scale);
            newMesh->setModelMatrix(model);

            if (shape) {
                newMesh->setPhysicsSystem(&physics->getPhysicsSystem());
                newMesh->setBaseShape(shape);

                JPH::ShapeRefC bodyShape = shape;
                glm::vec3 safeScale = glm::max(glm::abs(scale), glm::vec3(0.001f)) * glm::sign(scale);
                if (safeScale != glm::vec3(1.0f)) {
                    JPH::ScaledShapeSettings scaledSettings(shape, JPH::Vec3(safeScale.x, safeScale.y, safeScale.z));
                    auto scaledResult = scaledSettings.Create();
                    if (!scaledResult.HasError()) bodyShape = scaledResult.Get();
                }

                auto motionType = static_cast<JPH::EMotionType>(motionTypeInt);
                uint8_t layer = (motionType == JPH::EMotionType::Static) ? slate::Layers::NON_MOVING : slate::Layers::MOVING;

                JPH::BodyCreationSettings settings(
                    bodyShape,
                    JPH::RVec3(pos.x, pos.y, pos.z),
                    JPH::Quat(q.x, q.y, q.z, q.w),
                    motionType, layer);
                settings.mAllowDynamicOrKinematic = true;

                auto& bi = physics->getPhysicsSystem().GetBodyInterface();
                JPH::Body* body = bi.CreateBody(settings);
                if (body) {
                    bi.AddBody(body->GetID(), motionType == JPH::EMotionType::Static
                        ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);
                    newMesh->setBodyID(body->GetID());
                }
            }

            renderer->addMeshToScene(std::move(newMesh));

            if (hasPhysics && shape) {
                auto& added = renderer->getSceneMeshes().back();
                added->setPhysicsProperty("friction", friction);
                added->setPhysicsProperty("restitution", restitution);
                added->setPhysicsProperty("gravityFactor", gravityFactor);
                added->setPhysicsProperty("mass", mass);
            }
        };

        while (std::getline(file, line)) {
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.rfind("[[entity]]", 0) == 0) {
                if (hasEntity) {
                    spawnEntity();
                }
                hasEntity = true;
                currentMeshPath = "";
                currentEntityName = "";
                pos = glm::vec3(0.0f);
                rot = glm::vec3(0.0f);
                scale = glm::vec3(1.0f);
                albedo = glm::vec4(1.0f);
                roughness = 0.5f;
                metallic = 0.0f;
                ior = 1.5f;
                transmission = 0.0f;
                friction = 0.5f;
                restitution = 0.2f;
                gravityFactor = 1.0f;
                mass = 1.0f;
                motionTypeInt = 2;
                hasPhysics = false;
            } else if (line.rfind("name = ", 0) == 0 && hasEntity && currentEntityName.empty()) {
                size_t start = line.find('"');
                size_t end = line.rfind('"');
                if (start != std::string::npos && end != std::string::npos && end > start) {
                    currentEntityName = line.substr(start + 1, end - start - 1);
                }
            } else if (line.rfind("mesh_path = ", 0) == 0) {
                size_t start = line.find('"');
                size_t end = line.rfind('"');
                if (start != std::string::npos && end != std::string::npos && end > start) {
                    currentMeshPath = line.substr(start + 1, end - start - 1);
                }
            } else if (line.rfind("position = ", 0) == 0) {
                pos = parseVec3(line);
            } else if (line.rfind("rotation = ", 0) == 0) {
                rot = parseVec3(line);
            } else if (line.rfind("scale = ", 0) == 0) {
                scale = parseVec3(line);
            } else if (line.rfind("albedo = ", 0) == 0) {
                albedo = parseVec4(line);
            } else if (line.rfind("roughness = ", 0) == 0) {
                roughness = std::stof(line.substr(line.find('=') + 1));
            } else if (line.rfind("metallic = ", 0) == 0) {
                metallic = std::stof(line.substr(line.find('=') + 1));
            } else if (line.rfind("ior = ", 0) == 0) {
                ior = std::stof(line.substr(line.find('=') + 1));
            } else if (line.rfind("transmission = ", 0) == 0) {
                transmission = std::stof(line.substr(line.find('=') + 1));
            }
            else if (line.rfind("motion_type = ", 0) == 0) {
                motionTypeInt = std::stoi(line.substr(line.find('=') + 1));
                hasPhysics = true;
            } else if (line.rfind("friction = ", 0) == 0) {
                friction = std::stof(line.substr(line.find('=') + 1));
                hasPhysics = true;
            } else if (line.rfind("restitution = ", 0) == 0) {
                restitution = std::stof(line.substr(line.find('=') + 1));
                hasPhysics = true;
            } else if (line.rfind("gravity_factor = ", 0) == 0) {
                gravityFactor = std::stof(line.substr(line.find('=') + 1));
                hasPhysics = true;
            } else if (line.rfind("mass = ", 0) == 0) {
                mass = std::stof(line.substr(line.find('=') + 1));
                hasPhysics = true;
            }
        }

        if (hasEntity) {
            spawnEntity();
        }

        file.close();
        renderer->flushMaterialsToGPU();
        std::cout << "[serializer] scene successfully loaded from " << filepath << "\n";
        return true;
    }

}