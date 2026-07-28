#include "engine.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "resources/mesh_loader.hpp"
#include "ui/ui_manager.hpp"
#include "ui/ui_dockspace.hpp"
#include "ui/ui_button.hpp"
#include <stdexcept>
#include <SDL3/SDL_vulkan.h>
#include <iostream>

namespace slate {

    bool Engine::isPointInElement(glm::vec2 point, const std::shared_ptr<UIElement>& element) {
        if (!element) return false;
        glm::vec2 pos = element->getAbsolutePosition();
        glm::vec2 size = element->getSize();
        return (point.x >= pos.x && point.x <= pos.x + size.x &&
                point.y >= pos.y && point.y <= pos.y + size.y);
    }

    Engine::Engine() {
        initWindow();

        m_renderer = std::make_unique<VulkanRenderer>(m_window);
        m_renderer->init();

        // init ui manager
        m_uiManager = std::make_unique<UIManager>();
        m_uiManager->init(static_cast<float>(m_width), static_cast<float>(m_height));

        // dockspace layout
        auto mainDockSpace = std::make_shared<UIDockSpace>(
            "MainDockSpace",
            glm::vec2(0.0f),
            glm::vec2(m_width, m_height)
        );

        // top bar
        auto topBar = std::make_shared<UIElement>("TopBar", glm::vec2(0.0f), glm::vec2(0.0f));
        topBar->setDrawsBackground(true);
        topBar->setColor(glm::vec4(0.010f, 0.011f, 0.013f, 1.0f));
        mainDockSpace->addDockedChild(topBar, DockSlot::TopBar, 30.0f);

        auto importButton = std::make_shared<UIButton>(
            "importMeshBtn",
            glm::vec2(10.0f, 3.0f),
            glm::vec2(120.0f, 24.0f),
            [this]() {
                std::cout << "[UI] Importing mesh into viewport..." << std::endl;

                std::vector<Vertex> loadedVertices;
                std::vector<uint16_t> loadedIndices;
                std::vector<Material> loadedMaterials;

                if (MeshLoader::loadOBJ("models/cube.obj", loadedVertices, loadedIndices, loadedMaterials)) {
                    auto newMesh = std::make_unique<Mesh>(
                        static_cast<VulkanRenderer*>(m_renderer.get())->getDevice(),
                        static_cast<VulkanRenderer*>(m_renderer.get())->getPhysicalDevice(),
                        loadedVertices,
                        loadedIndices
                    );
                    static_cast<VulkanRenderer*>(m_renderer.get())->addMeshToScene(std::move(newMesh));
                    m_uiManager->markDirty();
                }
            }
        );
        topBar->addChild(importButton);

        // bottom bar
        auto bottomBar = std::make_shared<UIElement>("BottomBar", glm::vec2(0.0f), glm::vec2(0.0f));
        bottomBar->setDrawsBackground(true);
        bottomBar->setColor(glm::vec4(0.008f, 0.009f, 0.011f, 1.0f));
        mainDockSpace->addDockedChild(bottomBar, DockSlot::BottomBar, 25.0f);

        // left
        auto leftPanel = std::make_shared<UIElement>("HierarchyPanel", glm::vec2(0.0f), glm::vec2(0.0f));
        leftPanel->setDrawsBackground(true);
        leftPanel->setColor(glm::vec4(0.014f, 0.015f, 0.018f, 0.95f));
        mainDockSpace->addDockedChild(leftPanel, DockSlot::LeftSide, 280.0f);

        // right
        auto rightPanel = std::make_shared<UIElement>("InspectorPanel", glm::vec2(0.0f), glm::vec2(0.0f));
        rightPanel->setDrawsBackground(true);
        rightPanel->setColor(glm::vec4(0.014f, 0.015f, 0.018f, 0.95f));
        mainDockSpace->addDockedChild(rightPanel, DockSlot::RightSide, 320.0f);

        // viewport
        m_viewportPanel = std::make_shared<UIElement>("ViewportPanel", glm::vec2(0.0f), glm::vec2(0.0f));
        mainDockSpace->addDockedChild(m_viewportPanel, DockSlot::Center, 0.0f);

        m_uiManager->setRootElement(mainDockSpace);

        m_uiManager->markDirty();

        std::vector<Vertex> loadedVertices;
        std::vector<uint16_t> loadedIndices;
        std::vector<Material> loadedMaterials;

        if (MeshLoader::loadOBJ("models/test_obj.obj", loadedVertices, loadedIndices, loadedMaterials)) {
            static_cast<VulkanRenderer*>(m_renderer.get())->updateMaterials(loadedMaterials);

            auto newMesh = std::make_unique<Mesh>(
                static_cast<VulkanRenderer*>(m_renderer.get())->getDevice(),
                static_cast<VulkanRenderer*>(m_renderer.get())->getPhysicalDevice(),
                loadedVertices,
                loadedIndices
            );
            static_cast<VulkanRenderer*>(m_renderer.get())->addMeshToScene(std::move(newMesh));
        }

        m_cursorLocked = false;
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
            uint64_t currentTime = SDL_GetTicks();
            float deltaTime = (currentTime - m_lastTime) / 1000.0f;
            m_lastTime = currentTime;

            // update ui
            if (m_uiManager) {
                m_uiManager->update(deltaTime);
            }

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    shouldClose = true;
                }

                if (m_uiManager) {
                    m_uiManager->onEvent(event);
                }

                // toggle cursor lock
                if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    m_cursorLocked = false;
                    m_rightClickDragging = false;
                    m_viewportFocused = false;
                    SDL_SetWindowRelativeMouseMode(m_window, false);
                }

                // mouse button press handling
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    glm::vec2 mousePos{event.button.x, event.button.y};
                    bool clickInsideViewport = isPointInElement(mousePos, m_viewportPanel);

                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        if (clickInsideViewport) {
                            m_rightClickDragging = true;
                            m_viewportFocused = true;
                            m_cursorLocked = true;
                            SDL_SetWindowRelativeMouseMode(m_window, true);
                        }
                    } else if (event.button.button == SDL_BUTTON_LEFT) {
                        m_viewportFocused = clickInsideViewport;
                    }
                }

                // mouse button release handling
                if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        m_rightClickDragging = false;
                        m_cursorLocked = false;
                        SDL_SetWindowRelativeMouseMode(m_window, false);
                    }
                }

                if (event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                    int w = event.window.data1;
                    int h = event.window.data2;
                    m_renderer->onWindowResize(w, h);

                    if (m_uiManager) {
                        m_uiManager->setScreenSize(static_cast<float>(w), static_cast<float>(h));
                    }
                }

                // cursor lock
                if (m_cursorLocked && event.type == SDL_EVENT_MOUSE_MOTION) {
                    m_camera.processMouseMovement(event.motion.xrel, -event.motion.yrel);
                }
            }

            // wasd controls active
            if (m_viewportFocused || m_rightClickDragging) {
                const bool* keyboardState = SDL_GetKeyboardState(nullptr);
                m_camera.processKeyboard(keyboardState, deltaTime);
            }

            // rebuild ui
            if (m_uiManager && m_uiManager->isDirty()) {
                m_uiManager->rebuildGeometry();

                auto& vertices = m_uiManager->getVertexBuffer();
                auto& indices = m_uiManager->getIndexBuffer();

                static_cast<VulkanRenderer*>(m_renderer.get())->updateUIGeometryBuffers(vertices, indices);
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