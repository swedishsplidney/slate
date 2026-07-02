#include "engine.hpp"
#include <stdexcept>

namespace slate {

    Engine::Engine() {
        initWindow();
    }

    Engine::~Engine() {
        cleanup();
    }

    void Engine::initWindow() {
        // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

        // initialize glfw
        if (!glfwInit()) {
            throw std::runtime_error("failed to initialize glfw");
        }

        // tell glfw to not make an opengl context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // window resizing
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // create the window
        m_window = glfwCreateWindow(m_width, m_height, "slate engine", nullptr, nullptr);
        if (!m_window) {
            throw std::runtime_error("failed to create glfw window");
        }
    }

    void Engine::run() {
        mainLoop();
    }

    void Engine::mainLoop() {
        // keep running until the window is closed
        while (!glfwWindowShouldClose(m_window)) {
            glfwPollEvents();
        }
    }

    void Engine::cleanup() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

}