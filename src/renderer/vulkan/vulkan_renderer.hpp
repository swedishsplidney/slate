#pragma once

#include <memory>

#include "renderer/renderer.hpp"
#include "renderer/mesh.hpp"
#include "ui/ui_vertex.hpp"
#include "ui/ui_element.hpp"
#include "resources/mesh_loader.hpp"

#include <vulkan/vulkan.h>
#include <vector>
#include <SDL3/SDL.h>
#include <string>

namespace slate {

    struct QueueFamilyIndices {
        int graphicsFamily = -1;
        int presentFamily = -1;

        bool isComplete() const {
            return graphicsFamily >= 0 && presentFamily >= 0;
        }
    };

    struct GlobalUBO {
        alignas(16) glm::vec3 cameraPos;
        alignas(16) glm::vec3 lightDirection{0.5f, 1.0f, 0.5f};
        alignas(16) glm::vec3 lightColor{1.0f, 0.98f, 0.95f};
        alignas(4)  float lightIntensity = 2.5f;
    };

    struct UniformBufferObject {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    constexpr uint32_t MAX_MATERIALS = 1000;

    class VulkanRenderer : public Renderer {
        const int MAX_FRAMES_IN_FLIGHT = 2;
        const std::vector<const char*> DEVICE_EXTENSIONS = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

    public:
        VulkanRenderer(SDL_Window* window);
        ~VulkanRenderer() override;

        void init() override;
        void drawFrame(const glm::mat4& viewMatrix) override;
        void cleanup() override;

        VkDevice getDevice() const { return m_device; }
        VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

        void framebufferResized() { m_framebufferResized = true; }

        void addMeshToScene(std::unique_ptr<Mesh> mesh) { m_sceneMeshes.push_back(std::move(mesh)); }

        void updateUIGeometryBuffers(const std::vector<UIVertex>& vertices, const std::vector<uint16_t>& indices);

        void updateMaterials(const std::vector<Material>& materials);

    private:
        void createInstance();
        void createSurface();
        void pickPhysicalDevice();
        bool isDeviceSuitable(VkPhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void createLogicalDevice();
        void createSwapchain();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        void createImageViews();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();
        void createCommandPool();
        void createCommandBuffer();
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const glm::mat4& viewMatrix);
        void createDescriptorSetLayout();
        void createGraphicsPipeline();

        SDL_Window* m_window{nullptr};
        VkInstance m_instance{VK_NULL_HANDLE};
        VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
        VkSurfaceKHR m_surface{VK_NULL_HANDLE};
        VkDevice m_device{VK_NULL_HANDLE};
        VkQueue m_graphicsQueue{VK_NULL_HANDLE};
        VkQueue m_presentQueue{VK_NULL_HANDLE};
        VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
        std::vector<VkImage> m_swapchainImages;
        VkFormat m_swapchainImageFormat;
        VkExtent2D m_swapchainExtent;
        std::vector<VkImageView> m_swapchainImageViews;
        VkRenderPass m_renderPass{VK_NULL_HANDLE};
        std::vector<VkFramebuffer> m_swapchainFramebuffers;
        VkSemaphore m_imageAvailableSemaphore{VK_NULL_HANDLE};
        VkSemaphore m_renderFinishedSemaphore{VK_NULL_HANDLE};
        VkFence m_inFlightFence{VK_NULL_HANDLE};
        VkCommandPool m_commandPool{VK_NULL_HANDLE};
        VkCommandBuffer m_commandBuffer{VK_NULL_HANDLE};
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline{VK_NULL_HANDLE};

        std::vector<char> readFile(const std::string& filename);
        VkShaderModule createShaderModule(const std::vector<char>& code);

        std::vector<std::unique_ptr<Mesh>> m_sceneMeshes;

        VkPipelineLayout m_uiPipelineLayout{VK_NULL_HANDLE};
        VkPipeline m_uiGraphicsPipeline{VK_NULL_HANDLE};


        VkBuffer m_uiVertexBuffer{VK_NULL_HANDLE};
        VkDeviceMemory m_uiVertexBufferMemory{VK_NULL_HANDLE};

        VkBuffer m_uiIndexBuffer{VK_NULL_HANDLE};
        VkDeviceMemory m_uiIndexBufferMemory{VK_NULL_HANDLE};

        std::vector<UIVertex> m_uiVerticesMemory;
        std::vector<uint16_t> m_uiIndicesMemory;

        VkImage m_depthImage = VK_NULL_HANDLE;
        VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
        VkImageView m_depthImageView = VK_NULL_HANDLE;
        
        void createDepthResources();
        void createImage(uint32_t width, uint32_t height, VkFormat format,
                         VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, VkImage& image,
                         VkDeviceMemory& imageMemory);
        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

        bool m_uiDirty{true};

        bool m_framebufferResized = false;
        void recreateSwapchain();
        void cleanupSwapchain();

        void onWindowResize(int width, int height) override { m_framebufferResized = true; }

        void createUIGraphicsPipeline();

        void createUniformBuffers();
        void createDescriptorPoolAndSets();
        void updateUniformBuffer(uint32_t currentImage, const glm::vec3& cameraPos);

        std::vector<VkBuffer> m_uniformBuffers;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory;
        std::vector<void*> m_uniformBuffersMapped;

        std::vector<VkBuffer> m_materialBuffers;
        std::vector<VkDeviceMemory> m_materialBuffersMemory;
        std::vector<void*> m_materialBuffersMapped;

        std::vector<VkDescriptorSet> m_descriptorSets;

        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory
        );

        VkDescriptorSetLayout m_globalDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_materialDescriptorSetLayout = VK_NULL_HANDLE;

        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

        std::vector<VkDescriptorSet> m_globalDescriptorSets;
        std::vector<VkDescriptorSet> m_materialDescriptorSets;

        void createMaterialBuffers();

        uint32_t m_currentFrame = 0;

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    };

}