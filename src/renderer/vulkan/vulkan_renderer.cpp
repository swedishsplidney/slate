#include "vulkan_renderer.hpp"
#include <stdexcept>
#include <vector>
#include <iostream>

namespace slate {

    VulkanRenderer::VulkanRenderer(GLFWwindow* window) : m_window(window) {}

    VulkanRenderer::~VulkanRenderer() {
        cleanup();
    }

    void VulkanRenderer::init() {
        createInstance();
    }

    void VulkanRenderer::createInstance() {
        // define application info
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Slate Game";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Slate Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        // setup create info
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // get extensions
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // there are no global validation layers yet
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;

        // create the instance
        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vulkan instance!");
        }

        std::cout << "slate engine vulkan instance successfully created!\n";
    }

    void VulkanRenderer::drawFrame() {
        // will do later
    }

    void VulkanRenderer::cleanup() {
        if (m_instance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_instance, nullptr);
            m_instance = VK_NULL_HANDLE;
        }
    }

}