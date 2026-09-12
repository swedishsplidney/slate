namespace slate {

    struct alignas(16) MaterialGPU {
        glm::vec4 albedoFactor{1.0f};
        float roughnessFactor{0.5f};
        float metallicFactor{0.0f};
        float transmissionFactor{0.0f};
        float ior{1.45f};
        float aoFactor{1.0f};
        int hasTexture = 0;
        float padding[2];
    };

    struct Material {
        std::string name;
        uint32_t materialId = 0;
        std::string albedoTexturePath;
        MaterialGPU gpuData;

        VkImage textureImage = VK_NULL_HANDLE;
        VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
        VkImageView textureImageView = VK_NULL_HANDLE;
        VkSampler textureSampler = VK_NULL_HANDLE;

        std::vector<VkDescriptorSet> descriptorSets;
    };

}