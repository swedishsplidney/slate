#pragma once

#include "vertex.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace slate {

    class Mesh {
    public:
        Mesh(VkDevice device, VkPhysicalDevice physicalDevice,
            const std::vector<Vertex>& vertices,
            const std::vector<uint16_t>& indices);
        ~Mesh();

        // disable copying
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

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
    };

}