#include "engine.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "resources/mesh_loader.hpp"
#include "ui/ui_manager.hpp"
#include "ui/ui_dockspace.hpp"
#include "ui/ui_button.hpp"
#include "core/commands/file_commands.hpp"
#include "ui/ui_menu_bar.hpp"

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

        m_uiManager->loadFont("inter", "assets/fonts/Inter-4.1/InterVariable.ttf", 16.0f);
        m_uiManager->loadFont("jetbrains", "assets/fonts/JetBrains_Mono/JetBrainsMono-VariableFont_wght.ttf", 15.0f);

        m_commandRegistry = std::make_unique<CommandRegistry>();
        m_commandContext.renderer = static_cast<VulkanRenderer*>(m_renderer.get());
        m_commandContext.uiManager = m_uiManager.get();

        registerDefaultCommands();

        // dockspace layout
        auto mainDockSpace = std::make_shared<UIDockSpace>(
            "MainDockSpace",
            glm::vec2(0.0f),
            glm::vec2(m_width, m_height)
        );

        // top bar
        auto menuBar = std::make_shared<UIMenuBar>("MainMenuBar", glm::vec2(0.0f), glm::vec2(0.0f));
        menuBar->setUIManager(m_uiManager.get());

        auto fontLoader = m_uiManager->getFont("jetbrains");
        if (fontLoader) {
            menuBar->setFontLoader(fontLoader);

            auto vkRenderer = static_cast<VulkanRenderer*>(m_renderer.get());
            vkRenderer->createFontTexture(
                fontLoader->getAtlasBitmap().data(),
                fontLoader->getAtlasWidth(),
                fontLoader->getAtlasHeight()
            );
            vkRenderer->createUIDescriptorSet();
        }

        mainDockSpace->addDockedChild(menuBar, DockSlot::TopBar, 30.0f);

        menuBar->setOnCommandTriggered([this](const std::string& commandId) {
            m_commandRegistry->execute(commandId, m_commandContext);
        });

        MenuHeader fileMenu{"File", {
        {"Import Mesh...", "file.import_mesh", "Ctrl+I"},
        {"", "", "", true},
        {"Exit", "engine.exit", "Alt+F4"}
        }};

        MenuHeader editMenu{"Edit", {
        {"Undo", "editor.undo", "Ctrl+Z"},
        {"Redo", "editor.redo", "Ctrl+Y"}
        }};

        MenuHeader viewMenu{"View", {
        {"Reset Layout", "ui.reset_layout", ""}
        }};

        menuBar->addMenu(fileMenu);
        menuBar->addMenu(editMenu);
        menuBar->addMenu(viewMenu);

        auto importButton = std::make_shared<UIButton>(
            "importMeshBtn",
            glm::vec2(10.0f, 3.0f),
            glm::vec2(120.0f, 24.0f),
            [this]() {
                m_commandRegistry->execute("file.import_mesh", m_commandContext);
            }
        );

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

        m_commandRegistry->execute("file.import_mesh", m_commandContext, {"models/test_obj.obj"});

        m_cursorLocked = false;
    }

    void Engine::registerDefaultCommands() {
        m_commandRegistry->registerCommand("file.import_mesh", [](const CommandRegistry::CommandArgs& args) {
            std::string path = args.empty() ? "" : args[0];
            return std::make_unique<ImportMeshCommand>(path);
        });

        m_commandRegistry->registerCommand("editor.undo", [this](const CommandRegistry::CommandArgs&) {
            m_commandRegistry->getHistory().undo(m_commandContext);
            return nullptr;
        });
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

                // global shortcuts
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                        m_cursorLocked = false;
                        m_rightClickDragging = false;
                        m_viewportFocused = false;
                        SDL_SetWindowRelativeMouseMode(m_window, false);
                    }

                    // ctrl z
                    if ((event.key.mod & SDL_KMOD_CTRL) && event.key.scancode == SDL_SCANCODE_Z) {
                        m_commandRegistry->getHistory().undo(m_commandContext);
                    }
                }

                // mouse press handling
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

                // mouse release handling
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

                if (m_cursorLocked && event.type == SDL_EVENT_MOUSE_MOTION) {
                    m_camera.processMouseMovement(event.motion.xrel, -event.motion.yrel);
                }
            }

            if (m_viewportFocused || m_rightClickDragging) {
                const bool* keyboardState = SDL_GetKeyboardState(nullptr);
                m_camera.processKeyboard(keyboardState, deltaTime);
            }

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