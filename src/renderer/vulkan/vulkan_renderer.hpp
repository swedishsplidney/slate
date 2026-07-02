#pragma once

#include "renderer/renderer.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace slate {

    class VulkanRenderer : public Renderer {
    public:
        VulkanRenderer(GLFWwindow *window);
        ~VulkanRenderer() override;

        void init() override;
        void drawFrame() override;
        void cleanup() override;

    private:
        void createInstance();

        GLFWwindow *m_window{nullptr};
        VkInstance m_instance{VK_NULL_HANDLE};
    };

}