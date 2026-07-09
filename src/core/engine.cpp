#include "engine.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include <stdexcept>
#include <SDL3/SDL_vulkan.h>

namespace slate {

    Engine::Engine() {
        initWindow();

        m_renderer = std::make_unique<VulkanRenderer>(m_window);
        m_renderer->init();
    }

    Engine::~Engine() {
        cleanup();
    }

    void Engine::initWindow() {
        // init video subsystem
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error("failed to initialize SDL3: " + std::string(SDL_GetError()));
        }

        SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

        m_window = SDL_CreateWindow("Slate Engine", m_width, m_height, flags);
        if (!m_window) {
            throw std::runtime_error("failed to create SDL3 window: " + std::string(SDL_GetError()));
        }
    }

    void Engine::run() {
        mainLoop();
    }

    void Engine::mainLoop() {
        bool shouldClose = false;
        SDL_Event event;

        m_lastTime = SDL_GetTicks();

        SDL_SetWindowRelativeMouseMode(m_window, true);

        while (!shouldClose) {

            // calculate delta time each frame
            uint64_t currentTime = SDL_GetTicks();
            float deltaTime = (currentTime - m_lastTime) / 1000.0f;
            m_lastTime = currentTime;

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    shouldClose = true;
                }

                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                        m_cursorLocked = !m_cursorLocked;
                        SDL_SetWindowRelativeMouseMode(m_window, m_cursorLocked);
                    }
                }

                if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                    m_renderer->onWindowResize(event.window.data1, event.window.data2);
                }

                if (m_cursorLocked && event.type == SDL_EVENT_MOUSE_MOTION) {
                    m_camera.processMouseMovement(event.motion.xrel, -event.motion.yrel);
                }
            }

            if (m_cursorLocked) {
                const bool* keyboardState = SDL_GetKeyboardState(nullptr);
                m_camera.processKeyboard(keyboardState, deltaTime);
            }

            m_renderer->drawFrame(m_camera.getViewMatrix());
        }

        vkDeviceWaitIdle(static_cast<VulkanRenderer*>(m_renderer.get())->getDevice());
    }

    void Engine::cleanup() {
        if (m_renderer) {
            m_renderer.reset();
        }

        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }
        SDL_Quit();
    }

}