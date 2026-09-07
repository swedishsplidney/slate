#pragma once

#include "vertex.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace slate {

    class Mesh {
    public:
        Mesh(VkDevice device, VkPhysicalDevice physicalDevice,
            const std::vector<Vertex>& vertices,
            const std::vector<uint16_t>& indices,
            uint32_t materialId = 0,
            bool transparent = false,
            const std::string& name = "Mesh");

        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        uint32_t getMaterialId() const { return m_materialId; }
        bool isTransparent() const { return m_transparent; }

        const std::string& getName() const { return m_name; }
        void setName(const std::string& name) { m_name = name; }

        void setPhysicsSystem(JPH::PhysicsSystem* system) { m_physicsSystem = system; }

        void setBodyID(JPH::BodyID id) { m_bodyID = id; }
        JPH::BodyID getBodyID() const { return m_bodyID; }

        void setModelMatrix(const glm::mat4& matrix, bool updatePhysics = true) {
            m_modelMatrix = matrix;
            if (updatePhysics) {
                syncToPhysics();
            }
        }
        const glm::mat4& getModelMatrix() const { return m_modelMatrix; }

        void translate(const glm::vec3& delta) {
            m_modelMatrix = glm::translate(m_modelMatrix, delta);
            syncToPhysics();
        }

        void rotate(const glm::quat& rotation) {
            glm::mat4 translateToCenter = glm::translate(glm::mat4(1.0f), -m_geometricCenter);
            glm::mat4 rotateMat = glm::mat4_cast(rotation);
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), m_geometricCenter);

            m_modelMatrix = m_modelMatrix * translateBack * rotateMat * translateToCenter;
            syncToPhysics();
        }

        glm::vec3 getGeometricCenter() const { return m_geometricCenter; }

        const std::vector<Vertex>& getVertices() const { return m_vertices; }
        const std::vector<uint16_t>& getIndices() const { return m_indices; }

        void setTransparent(bool transparent) { m_transparent = transparent; }

        void setPath(const std::string& path) { m_filePath = path; }
        const std::string& getPath() const { return m_filePath; }



        void setPhysicsProperty(const std::string& propertyName, float value) {
            if (!m_physicsSystem || m_bodyID.IsInvalid()) return;
            auto& bodyInterface = m_physicsSystem->GetBodyInterface();

            if (propertyName == "friction") {
                bodyInterface.SetFriction(m_bodyID, value);
            }
            else if (propertyName == "restitution" || propertyName == "bounciness") {
                bodyInterface.SetRestitution(m_bodyID, value);
            }
            else if (propertyName == "gravityFactor") {
                bodyInterface.SetGravityFactor(m_bodyID, value);
            }
            else if (propertyName == "motionType" || propertyName == "bodyType") {
                JPH::EMotionType motionType = JPH::EMotionType::Dynamic;
                int typeInt = static_cast<int>(value);
                if (typeInt == 0) motionType = JPH::EMotionType::Static;
                else if (typeInt == 1) motionType = JPH::EMotionType::Kinematic;
                else if (typeInt == 2) motionType = JPH::EMotionType::Dynamic;

                bodyInterface.SetMotionType(m_bodyID, motionType, JPH::EActivation::Activate);
            }
            else if (propertyName == "mass") {
                JPH::BodyLockWrite lock(m_physicsSystem->GetBodyLockInterface(), m_bodyID);
                if (lock.Succeeded()) {
                    JPH::Body& body = lock.GetBody();
                    if (!body.IsStatic()) {
                        if (body.GetMotionProperties()) {
                            body.GetMotionProperties()->ScaleToMass(value);
                        }
                    } else {
                        std::cout << "[physics warning] Cannot change mass of a static body.\n";
                    }
                }
            }
        }

    private:
        void createVertexBuffer(VkPhysicalDevice physicalDevice, const std::vector<Vertex>& vertices);
        void createIndexBuffer(VkPhysicalDevice physicalDevice, const std::vector<uint16_t>& indices);

        VkDevice m_device = VK_NULL_HANDLE;
        VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
        uint32_t m_vertexCount = 0;

        VkBuffer m_indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;
        uint32_t m_indexCount = 0;

        uint32_t m_materialId = 0;
        bool m_transparent = false;

        std::string m_name;

        glm::mat4 m_modelMatrix{1.0f};
        glm::vec3 m_geometricCenter{0.0f};

        std::vector<Vertex> m_vertices;
        std::vector<uint16_t> m_indices;

        std::string m_filePath;

        void syncToPhysics() {
            if (m_physicsSystem && !m_bodyID.IsInvalid()) {
                auto& bodyInterface = m_physicsSystem->GetBodyInterface();

                glm::vec3 pos = glm::vec3(m_modelMatrix[3]);
                glm::quat rot = glm::quat_cast(m_modelMatrix);

                JPH::RVec3 joltPos(pos.x, pos.y, pos.z);
                JPH::Quat joltRot(rot.x, rot.y, rot.z, rot.w);

                if (bodyInterface.GetMotionType(m_bodyID) == JPH::EMotionType::Static) {
                    bodyInterface.SetPositionAndRotation(m_bodyID, joltPos, joltRot, JPH::EActivation::DontActivate);
                    return;
                }

                bodyInterface.SetPositionAndRotation(m_bodyID, joltPos, joltRot, JPH::EActivation::Activate);

                bodyInterface.SetLinearVelocity(m_bodyID, JPH::Vec3::sZero());
                bodyInterface.SetAngularVelocity(m_bodyID, JPH::Vec3::sZero());
            }
        }

        JPH::PhysicsSystem* m_physicsSystem{nullptr};
        JPH::BodyID m_bodyID;
    };

}