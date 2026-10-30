#pragma once

#include <memory>
#include <array>
#include <cstddef>

#include "renderer/renderer.hpp"
#include "renderer/mesh.hpp"
#include "renderer/light.hpp"
#include "renderer/shadow.hpp"
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

    struct alignas(16) GlobalUBO {
        glm::vec3 cameraPos;
        float exposure = 1.0f;

        glm::vec3 lightDirection{0.0f, 1.0f, 0.0f};
        int32_t lightType = 0;

        glm::vec3 lightColor{1.0f, 0.95f, 0.9f};
        float lightIntensity = 3.0f;

        glm::vec3 lightPos{0.0f};
        float lightRange = 25.0f;

        glm::vec4 lightParams{1.0f, 0.02f, 0.0f, 0.0f};

        glm::vec4 ambientCube[6]{};
        glm::mat4 lightSpaceMatrix;

        glm::mat4 shadowMatrices[6];
        glm::vec4 cameraForward;
        glm::vec4 cascadeSplits;
        glm::vec4 cascadeTexelWorld;
        glm::vec4 cascadeDepthRange;
        glm::vec4 shadowParams0;
        glm::vec4 shadowParams1;
        glm::vec4 shadowParams2;
        glm::vec4 shadowParams3;
        glm::vec4 shadowParams4;
    };
    static_assert(offsetof(GlobalUBO, lightSpaceMatrix) + sizeof(glm::mat4) == 240, "GlobalUBO prefix must not move");
    static_assert(sizeof(GlobalUBO) == 768, "GlobalUBO must match the std140 layout in pbr.frag / shadow.vert");

    struct ShadowPushConstants {
        glm::mat4 modelMatrix;
        uint32_t materialId;
        uint32_t layer;
    };
    static_assert(sizeof(ShadowPushConstants) <= 128, "shadow push constants exceed the guaranteed minimum limit");

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
        void drawFrame(const glm::mat4& viewMatrix, glm::vec2 viewportOffset, glm::vec2 viewportSize) override;
        void cleanup() override;

        VkDevice getDevice() const { return m_device; }
        VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }

        void framebufferResized() { m_framebufferResized = true; }

        uint32_t addMeshToScene(std::unique_ptr<Mesh> mesh) {
            m_sceneMeshes.push_back(std::move(mesh));
            return static_cast<uint32_t>(m_sceneMeshes.size() - 1);
        }

        void updateUIGeometryBuffers(const std::vector<UIVertex>& vertices, const std::vector<UIIndex>& indices);

        void updateMaterials(const std::vector<Material>& materials);

        void createFontTexture(const unsigned char* pixels = nullptr, uint32_t width = 0, uint32_t height = 0);
        void createUIDescriptorSet();

        void createMaterialDescriptorSets(Material& mat);

        std::vector<Material> m_globalMaterials;

        std::vector<Material>& getGlobalMaterials() { return m_globalMaterials; }

        void flushMaterialsToGPU();

        void freeMaterialDescriptorSets(Material& mat) {
            if (!mat.descriptorSets.empty() && m_descriptorPool != VK_NULL_HANDLE) {
                vkFreeDescriptorSets(m_device, m_descriptorPool,
                                     static_cast<uint32_t>(mat.descriptorSets.size()),
                                     mat.descriptorSets.data());
            }
            mat.descriptorSets.clear();
        }

        void removeLastMeshFromScene() {
            if (!m_sceneMeshes.empty()) {
                vkDeviceWaitIdle(m_device);
                m_sceneMeshes.pop_back();
            }
        }

        void clearScene() {
            vkDeviceWaitIdle(m_device);
            m_sceneMeshes.clear();
            for (auto& mat : m_globalMaterials) freeMaterialDescriptorSets(mat);
            m_globalMaterials.clear();
        }

        void popGlobalMaterials(size_t count) {
            if (count <= m_globalMaterials.size()) {
                vkDeviceWaitIdle(m_device);
                for (auto it = m_globalMaterials.end() - count; it != m_globalMaterials.end(); ++it) {
                    freeMaterialDescriptorSets(*it);
                }
                m_globalMaterials.erase(m_globalMaterials.end() - count, m_globalMaterials.end());
            }
        }

        bool isGridEnabled() const { return m_showGrid; }
        void toggleGrid() { m_showGrid = !m_showGrid; }

        std::unique_ptr<Mesh> m_gridMesh;
        std::unique_ptr<Mesh> m_axesMesh;

        const std::vector<std::unique_ptr<Mesh>>& getSceneMeshes() const { return m_sceneMeshes; }
        std::vector<std::unique_ptr<Mesh>>& getSceneMeshes() { return m_sceneMeshes; }

        void setSelectedMeshIndex(int index) { m_selectedMeshIndex = index; }

        enum class GizmoMode {
            Translate = 0,
            Rotate = 1
        };

        void setGizmoMode(GizmoMode mode);
        GizmoMode getGizmoMode() const { return m_gizmoMode; }

        void setViewportBounds(glm::vec2 offset, glm::vec2 size) {
            m_viewportOffset = offset;
            m_viewportSize = size;
        }

        ShadowSettings& getShadowSettings() { return m_shadowSettings; }
        const ShadowSettings& getShadowSettings() const { return m_shadowSettings; }
        void setShadowSettings(const ShadowSettings& settings) { m_shadowSettings = settings; }

        void setCameraProjection(float fovYDegrees, float nearPlane, float farPlane) {
            m_cameraFovY = glm::radians(fovYDegrees);
            m_cameraNear = nearPlane;
            m_cameraFar = farPlane;
        }

        void setLight(const SceneLight& light) { m_sceneLight = light; }
        SceneLight& getLight() { return m_sceneLight; }
        const SceneLight& getLight() const { return m_sceneLight; }

        std::vector<DynamicPointLight> m_dynamicLights;
        std::vector<DynamicPointLight>& getDynamicLights() { return m_dynamicLights; }

        bool loadTexture(const std::string& filepath, VkImage& outImage, VkDeviceMemory& outMemory, VkImageView& outImageView, VkSampler& outSampler);
        bool importAndApplyTexture(const std::string& filepath, uint32_t materialIndex);
        bool importAndApplyNormalTexture(const std::string& filepath, uint32_t materialIndex);
        bool importAndApplyOrmTexture(const std::string& filepath, uint32_t materialIndex);

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
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const glm::mat4& viewMatrix, glm::vec2 viewportOffset, glm::vec2 viewportSize);
        void createDescriptorSetLayout();
        void createGraphicsPipeline();

        void createShadowResources();
        void createShadowRenderPass();
        void createShadowFramebuffer();
        void createShadowPipeline();
        void destroyShadowResources();
        void recreateShadowResources();
        void recordShadowPasses(VkCommandBuffer commandBuffer);

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
        VkDescriptorSetLayout m_uiDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorSet m_uiDescriptorSet = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkPipeline m_graphicsPipeline{VK_NULL_HANDLE};
        VkPipeline m_transparentPipeline{VK_NULL_HANDLE};

        ShadowSettings m_shadowSettings;
        uint32_t m_shadowResolution{0};
        bool m_shadowActive{false};
        uint32_t m_shadowLayerCount{0};
        std::array<glm::mat4, SHADOW_MAP_LAYERS> m_shadowLayerMatrices{};
        std::vector<glm::vec4> m_shadowCasterSpheres;

        float m_cameraFovY{glm::radians(45.0f)};
        float m_cameraNear{0.001f};
        float m_cameraFar{1000.0f};

        VkRenderPass m_shadowRenderPass{VK_NULL_HANDLE};
        std::array<VkFramebuffer, SHADOW_MAP_LAYERS> m_shadowFramebuffers{};
        std::array<VkImageView, SHADOW_MAP_LAYERS> m_shadowLayerViews{};
        VkImage m_shadowMapImage{VK_NULL_HANDLE};
        VkDeviceMemory m_shadowMapImageMemory{VK_NULL_HANDLE};
        VkImageView m_shadowMapImageView{VK_NULL_HANDLE};
        VkSampler m_shadowMapSampler{VK_NULL_HANDLE};
        VkSampler m_shadowDepthSampler{VK_NULL_HANDLE};
        VkPipeline m_shadowPipeline{VK_NULL_HANDLE};
        VkPipelineLayout m_shadowPipelineLayout{VK_NULL_HANDLE};

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
        std::vector<UIIndex> m_uiIndicesMemory;

        VkImage m_depthImage = VK_NULL_HANDLE;
        VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
        VkImageView m_depthImageView = VK_NULL_HANDLE;
        VkFormat m_depthFormat = VK_FORMAT_D32_SFLOAT;

        void createDepthResources();
        void createImage(uint32_t width, uint32_t height, VkFormat format,
                         VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, VkImage& image,
                         VkDeviceMemory& imageMemory, uint32_t arrayLayers = 1);
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
        void updateUniformBuffer(const glm::vec3& cameraPos, const glm::mat4& viewMatrix, float aspect);

        std::vector<VkBuffer> m_uniformBuffers;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory;
        std::vector<void*> m_uniformBuffersMapped;

        std::vector<VkBuffer> m_materialBuffers;
        std::vector<VkDeviceMemory> m_materialBuffersMemory;
        std::vector<void*> m_materialBuffersMapped;

        std::vector<VkBuffer> m_dynamicLightBuffers;
        std::vector<VkDeviceMemory> m_dynamicLightBuffersMemory;
        std::vector<void*> m_dynamicLightBuffersMapped;

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
        void createDynamicLightBuffers();
        void updateDynamicLightBuffer();

        uint32_t m_currentFrame = 0;

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        VkImage m_fontImage = VK_NULL_HANDLE;
        VkDeviceMemory m_fontImageMemory = VK_NULL_HANDLE;
        VkImageView m_fontImageView = VK_NULL_HANDLE;
        VkSampler m_fontSampler = VK_NULL_HANDLE;

        void createUIDescriptorSetLayout();

        VkImage m_sceneColorImage = VK_NULL_HANDLE;
        VkDeviceMemory m_sceneColorMemory = VK_NULL_HANDLE;
        VkImageView m_sceneColorImageView = VK_NULL_HANDLE;
        VkSampler m_sceneColorSampler = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> m_sceneColorFramebuffers;

        void createSceneColorResources();
        void cleanupSceneColorResources();

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);

        VkRenderPass m_transparentRenderPass{VK_NULL_HANDLE};
        void createTransparentRenderPass();

        bool m_showGrid{true};
        GizmoMode m_gizmoMode{GizmoMode::Translate};

        void createGridMesh();
        VkPipeline m_gridPipeline{VK_NULL_HANDLE};
        void createGridPipeline();

        std::unique_ptr<Mesh> m_gizmoMesh;
        VkPipeline m_gizmoPipeline{VK_NULL_HANDLE};

        void createGizmoMesh();
        void createGizmoPipeline();

        size_t m_selectedMeshIndex = 0;

        glm::vec2 m_viewportOffset{0.0f};
        glm::vec2 m_viewportSize{0.0f};

        SceneLight m_sceneLight;

        VkImage m_textureImage = VK_NULL_HANDLE;
        VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
        VkImageView m_textureImageView = VK_NULL_HANDLE;
        VkSampler m_textureSampler = VK_NULL_HANDLE;

        VkImage m_defaultNormalImage = VK_NULL_HANDLE;
        VkDeviceMemory m_defaultNormalImageMemory = VK_NULL_HANDLE;
        VkImageView m_defaultNormalImageView = VK_NULL_HANDLE;

        VkImage m_defaultOrmImage = VK_NULL_HANDLE;
        VkDeviceMemory m_defaultOrmImageMemory = VK_NULL_HANDLE;
        VkImageView m_defaultOrmImageView = VK_NULL_HANDLE;

        VkImage m_defaultEmissiveImage = VK_NULL_HANDLE;
        VkDeviceMemory m_defaultEmissiveImageMemory = VK_NULL_HANDLE;
        VkImageView m_defaultEmissiveImageView = VK_NULL_HANDLE;

        void createDefaultTexture();
        void createTextureImageFromData(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height, VkFormat format, VkImage& image, VkDeviceMemory& memory);
    };

}