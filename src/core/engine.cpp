#include "engine.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "resources/mesh_loader.hpp"
#include "ui/ui_button.hpp"
#include <stdexcept>
#include <SDL3/SDL_vulkan.h>
#include <iostream>

namespace slate {

    Engine::Engine() {
        initWindow();

        m_renderer = std::make_unique<VulkanRenderer>(m_window);
        m_renderer->init();

        //  init ui container
        m_uiRoot = std::make_shared<UIElement>("RootCanvas", glm::vec2(0.0f), glm::vec2(m_width, m_height));

        auto importButton = std::make_shared<UIButton>(
            "ImportMeshBtn",
            glm::vec2(20.0f, 20.0f),
            glm::vec2(160.0f, 50.0f),
            [this]() {
                std::cout << "[UI Notification] Clicked! Importing target mesh..." << std::endl;

                std::vector<Vertex> loadedVertices;
                std::vector<uint16_t> loadedIndices;

                // fire tinyobj loader
                if (MeshLoader::loadOBJ("models/cube.obj", loadedVertices, loadedIndices)) {
                    auto newMesh = std::make_unique<Mesh>(
                        static_cast<VulkanRenderer*>(m_renderer.get())->getDevice(),
                        static_cast<VulkanRenderer*>(m_renderer.get())->getPhysicalDevice(),
                        loadedVertices,
                        loadedIndices
                    );
                    static_cast<VulkanRenderer*>(m_renderer.get())->addMeshToScene(std::move(newMesh));
                }
            }
        );

        m_uiRoot->addChild(importButton);
        m_cursorLocked = true;
    }

    Engine::~Engine() {
        cleanup();
    }

    void Engine::initWindow() {
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
        SDL_SetWindowRelativeMouseMode(m_window, m_cursorLocked);

        while (!shouldClose) {
            // Calculate delta time each frame
            uint64_t currentTime = SDL_GetTicks();
            float deltaTime = (currentTime - m_lastTime) / 1000.0f;
            m_lastTime = currentTime;

            // Update UI elements first (scalable tick rates)
            if (m_uiRoot) {
                m_uiRoot->update(deltaTime);
            }

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    shouldClose = true;
                }

                if (m_uiRoot) {
                    m_uiRoot->onEvent(event);
                }

                // escape to unlock cursor
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                        m_cursorLocked = false;
                        SDL_SetWindowRelativeMouseMode(m_window, m_cursorLocked);
                    }
                }

                // click to re-lock
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT && !m_cursorLocked) {
                        m_cursorLocked = true;
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