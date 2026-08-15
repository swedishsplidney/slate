#pragma once

#include "vertex.hpp"
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace slate {

    class Mesh {
    public:
        Mesh(VkDevice device, VkPhysicalDevice physicalDevice,
            const std::vector<Vertex>& vertices,
            const std::vector<uint16_t>& indices,
            uint32_t materialId = 0,
            bool transparent = false);
        ~Mesh();

        // disable copying
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        uint32_t getMaterialId() const { return m_materialId; }

        bool isTransparent() const { return m_transparent; }

        void setModelMatrix(const glm::mat4& matrix) { m_modelMatrix = matrix; }
        const glm::mat4& getModelMatrix() const { return m_modelMatrix; }

        void translate(const glm::vec3& delta) {
            m_modelMatrix = glm::translate(m_modelMatrix, delta);
        }

        glm::vec3 getGeometricCenter() const { return m_geometricCenter; }

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

        glm::mat4 m_modelMatrix{1.0f};
        glm::vec3 m_geometricCenter{0.0f};
    };

}