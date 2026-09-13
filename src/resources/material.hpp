namespace slate {

    struct alignas(16) MaterialGPU {
        glm::vec4 albedoFactor{1.0f, 1.0f, 1.0f, 1.0f};
        glm::vec4 emissiveFactor{0.0f, 0.0f, 0.0f, 1.0f};

        float roughnessFactor{0.5f};
        float metallicFactor{0.0f};
        float transmissionFactor{0.0f};
        float ior{1.45f};

        float aoFactor{1.0f};
        float rimIntensity{0.0f};
        float rimExponent{2.0f};
        float alphaCutoff{0.0f};

        int hasAlbedoTexture = 0;
        int hasNormalTexture = 0;
        int hasOrmTexture = 0;
        int hasEmissiveTexture = 0;
    };
    static_assert(sizeof(MaterialGPU) == 80, "MaterialGPU must match pbr.frag std430 layout");

    struct Material {
        std::string name;
        uint32_t materialId = 0;

        std::string albedoTexturePath;
        std::string normalTexturePath;
        std::string ormTexturePath;
        std::string emissiveTexturePath;

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

        // emmissive:
        VkImage emissiveImage = VK_NULL_HANDLE;
        VkDeviceMemory emissiveImageMemory = VK_NULL_HANDLE;
        VkImageView emissiveImageView = VK_NULL_HANDLE;

        VkSampler textureSampler = VK_NULL_HANDLE;
        MaterialGPU gpuData;
        std::vector<VkDescriptorSet> descriptorSets;
    };

}