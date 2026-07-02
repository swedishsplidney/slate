#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace slate {

    class Engine {
    public:
        Engine();
        ~Engine();

        // prevent copying the engine
        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        void run();

    private:
        void initWindow();
        void mainLoop();
        void cleanup();

        GLFWwindow* m_window{nullptr};
        const int m_width{800};
        const int m_height{600};
    };

}