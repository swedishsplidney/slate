#pragma once
#include <string>

namespace slate {
    class VulkanRenderer;
    class PhysicsEngine;

    class Serializer {
    public:
        static bool saveScene(const std::string& filepath, VulkanRenderer* renderer);
        static bool loadScene(const std::string& filepath, VulkanRenderer* renderer, PhysicsEngine* physics = nullptr);
    };
}