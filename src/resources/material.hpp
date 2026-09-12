namespace slate {

    struct alignas(16) MaterialGPU {
        glm::vec4 albedoFactor{1.0f};
        float roughnessFactor{0.5f};
        float metallicFactor{0.0f};
        float transmissionFactor{0.0f};
        float ior{1.45f};
        float aoFactor{1.0f};

        int hasAlbedoTexture = 0;
        int hasNormalTexture = 0;
        int hasOrmTexture = 0;
        float padding[1];
    };

    struct Material {
        std::string name;
        uint32_t materialId = 0;

        std::string albedoTexturePath;
        std::string normalTexturePath;
        std::string ormTexturePath;

        // albedo
        VkImage albedoImage = VK_NULL_HANDLE;
        VkDeviceMemory albedoImageMemory = VK_NULL_HANDLE;
        VkImageView albedoImageView = VK_NULL_HANDLE;

        // normal
        VkImage normalImage = VK_NULL_HANDLE;
        VkDeviceMemory normalImageMemory = VK_NULL_HANDLE;
        VkImageView normalImageView = VK_NULL_HANDLE;

        // occlusion, roughness, metallic
        VkImage ormImage = VK_NULL_HANDLE;
        VkDeviceMemory ormImageMemory = VK_NULL_HANDLE;
        VkImageView ormImageView = VK_NULL_HANDLE;

        VkSampler textureSampler = VK_NULL_HANDLE;
        MaterialGPU gpuData;

        std::vector<VkDescriptorSet> descriptorSets;
    };

}