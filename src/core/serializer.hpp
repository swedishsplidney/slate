#pragma once
#include <string>

namespace slate {
    class VulkanRenderer;

    class Serializer {
    public:
        static bool saveScene(const std::string& filepath, VulkanRenderer* renderer);

        static bool loadScene(const std::string& filepath, VulkanRenderer* renderer);
    };
}