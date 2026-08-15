#include "engine.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "resources/mesh_loader.hpp"
#include "ui/ui_manager.hpp"
#include "ui/ui_dockspace.hpp"
#include "ui/ui_button.hpp"
#include "core/commands/file_commands.hpp"
#include "core/commands/editor_commands.hpp"
#include "core/commands/translate_commands.hpp"
#include "ui/ui_menu_bar.hpp"

#include <stdexcept>
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <algorithm>

namespace slate {

    Ray Engine::screenPointToRay(glm::vec2 mousePos) {
        glm::vec2 vpOffset(0.0f);
        glm::vec2 vpSize(static_cast<float>(m_width), static_cast<float>(m_height));

        float localX = mousePos.x - vpOffset.x;
        float localY = mousePos.y - vpOffset.y;

        float ndcX = (2.0f * localX) / vpSize.x - 1.0f;
        float ndcY = (2.0f * localY) / vpSize.y - 1.0f;

        float aspect = vpSize.x / (vpSize.y > 0.0f ? vpSize.y : 1.0f);
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
        proj[1][1] *= -1.0f;
        glm::mat4 viewMatrix = m_camera.getViewMatrix();

        glm::mat4 invProj = glm::inverse(proj);
        glm::mat4 invView = glm::inverse(viewMatrix);

        glm::vec4 rayClip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
        glm::vec4 rayEye = invProj * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

        Ray ray;
        ray.direction = glm::normalize(glm::vec3(invView * rayEye));
        ray.origin = glm::vec3(invView[3]);
        return ray;
    }

    bool intersectRayPlane(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                           const glm::vec3& planeOrigin, const glm::vec3& planeNormal,
                           glm::vec3& outIntersection) {
        float denom = glm::dot(rayDir, planeNormal);
        if (std::abs(denom) > 0.0001f) {
            float t = glm::dot(planeOrigin - rayOrigin, planeNormal) / denom;
            if (t >= 0.0f) {
                outIntersection = rayOrigin + t * rayDir;
                return true;
            }
        }
        return false;
    }

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
                {"Toggle Grid", "view.toggle_grid", ""},
                {"Translate Gizmo", "editor.set_gizmo_translate", ""},
                {"Rotate Gizmo", "editor.set_gizmo_rotate", ""},
                {"", "", "", true},
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

        m_commandRegistry->registerCommand("view.toggle_grid", [](const CommandRegistry::CommandArgs&) {
            return std::make_unique<ToggleGridCommand>();
        });

        m_commandRegistry->registerCommand("editor.set_gizmo_translate", [](const CommandRegistry::CommandArgs&) {
            return std::make_unique<SetGizmoModeCommand>(SetGizmoModeCommand::Mode::Translate);
        });

        m_commandRegistry->registerCommand("editor.set_gizmo_rotate", [](const CommandRegistry::CommandArgs&) {
            return std::make_unique<SetGizmoModeCommand>(SetGizmoModeCommand::Mode::Rotate);
        });

        m_commandRegistry->registerCommand("editor.undo", [this](const CommandRegistry::CommandArgs&) {
            m_commandRegistry->getHistory().undo(m_commandContext);
            return nullptr;
        });

        m_commandRegistry->registerCommand("editor.translate_mesh", [this](const CommandRegistry::CommandArgs& args) {
            glm::vec3 delta(0.0f, 0.5f, 0.0f);
            return std::make_unique<TranslateMeshCommand>(m_selectedMeshIndex, delta);
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

    float distToSegment(glm::vec2 p, glm::vec2 a, glm::vec2 b) {
        glm::vec2 pa = p - a, ba = b - a;
        float h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
        return glm::length(pa - ba * h);
    }

    glm::vec2 Engine::worldToScreen(const glm::vec3& worldPos) {
        glm::vec2 vpOffset(0.0f);
        glm::vec2 vpSize(static_cast<float>(m_width), static_cast<float>(m_height));
        float aspect = vpSize.x / (vpSize.y > 0.0f ? vpSize.y : 1.0f);

        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
        proj[1][1] *= -1.0f;
        glm::mat4 view = m_camera.getViewMatrix();

        glm::vec4 clipPos = proj * view * glm::vec4(worldPos, 1.0f);
        if (clipPos.w <= 0.001f) return glm::vec2(-99999.0f);

        glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;

        float x = vpOffset.x + (ndc.x * 0.5f + 0.5f) * vpSize.x;
        float y = vpOffset.y + (ndc.y * 0.5f + 0.5f) * vpSize.y;
        return glm::vec2(x, y);
    }

    int Engine::checkGizmoHit(const glm::vec2& mousePos) {
        auto& sceneMeshes = static_cast<VulkanRenderer*>(m_renderer.get())->getSceneMeshes();
        if (sceneMeshes.empty() || m_selectedMeshIndex >= sceneMeshes.size() || !sceneMeshes[m_selectedMeshIndex]) {
            return -1;
        }

        auto& mesh = sceneMeshes[m_selectedMeshIndex];
        glm::vec3 gizmoCenter = glm::vec3(mesh->getModelMatrix() * glm::vec4(mesh->getGeometricCenter(), 1.0f));

        float dist = glm::distance(m_camera.getPosition(), gizmoCenter);
        float t = dist / (dist + 8.0f);
        float gizmoScale = glm::mix(0.05f, 1.0f, t);

        glm::vec2 vpOffset(0.0f);
        glm::vec2 vpSize(static_cast<float>(m_width), static_cast<float>(m_height));
        float aspect = vpSize.x / (vpSize.y > 0.0f ? vpSize.y : 1.0f);

        glm::mat4 proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
        proj[1][1] *= -1.0f;
        glm::mat4 view = m_camera.getViewMatrix();

        auto getValidScreenPos = [&](const glm::vec3& worldPos, glm::vec2& outScreen) {
            glm::vec4 clipPos = proj * view * glm::vec4(worldPos, 1.0f);
            if (clipPos.w <= 0.001f) return false;

            glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;
            float x = vpOffset.x + (ndc.x * 0.5f + 0.5f) * vpSize.x;
            float y = vpOffset.y + (ndc.y * 0.5f + 0.5f) * vpSize.y;
            outScreen = glm::vec2(x, y);
            return true;
        };

        glm::vec2 screenOrigin;
        if (!getValidScreenPos(gizmoCenter, screenOrigin)) {
            return -1;
        }

        float distX = 99999.0f, distY = 99999.0f, distZ = 99999.0f;
        glm::vec2 screenPosX, screenPosY, screenPosZ;
        const float minScreenLengthForHit = 5.0f;

        if (getValidScreenPos(gizmoCenter + glm::vec3(gizmoScale, 0.0f, 0.0f), screenPosX)) {
            if (glm::length(screenPosX - screenOrigin) >= minScreenLengthForHit) {
                distX = distToSegment(mousePos, screenOrigin, screenPosX);
            }
        }

        if (getValidScreenPos(gizmoCenter + glm::vec3(0.0f, gizmoScale, 0.0f), screenPosY)) {
            if (glm::length(screenPosY - screenOrigin) >= minScreenLengthForHit) {
                distY = distToSegment(mousePos, screenOrigin, screenPosY);
            }
        }

        if (getValidScreenPos(gizmoCenter + glm::vec3(0.0f, 0.0f, gizmoScale), screenPosZ)) {
            if (glm::length(screenPosZ - screenOrigin) >= minScreenLengthForHit) {
                distZ = distToSegment(mousePos, screenOrigin, screenPosZ);
            }
        }

        float threshold = 12.0f;
        float minDist = std::min({distX, distY, distZ});

        if (minDist <= threshold) {
            if (minDist == distX) return 0;
            if (minDist == distY) return 1;
            if (minDist == distZ) return 2;
        }

        return -1;
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

            bool clickInsideViewport = false;
            glm::vec2 mousePos(0.0f);

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

                    if (!(event.key.mod & SDL_KMOD_CTRL) && event.key.scancode == SDL_SCANCODE_G) {
                        m_commandRegistry->execute("editor.translate_mesh", m_commandContext);
                    }
                }

                // capture mouse position and viewport bounds on press
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    float mx, my;
                    SDL_GetMouseState(&mx, &my);
                    mousePos = glm::vec2(mx, my);
                    clickInsideViewport = isPointInElement(mousePos, m_viewportPanel);

                    if (event.button.button == SDL_BUTTON_RIGHT && clickInsideViewport) {
                        m_rightClickDragging = true;
                        m_cursorLocked = true;
                        SDL_SetWindowRelativeMouseMode(m_window, true);
                    } else if (event.button.button == SDL_BUTTON_LEFT && clickInsideViewport) {
                        int hitAxis = checkGizmoHit(mousePos);
                        std::cout << "left click at (" << mousePos.x << ", " << mousePos.y << ") -> hit axis: " << hitAxis << "\n";

                        if (hitAxis != -1) {
                            auto& sceneMeshes = static_cast<VulkanRenderer*>(m_renderer.get())->getSceneMeshes();
                            if (!sceneMeshes.empty() && m_selectedMeshIndex < sceneMeshes.size() && sceneMeshes[m_selectedMeshIndex]) {
                                m_activeGizmoAxis = hitAxis;
                                m_gizmoDragStartPos = mousePos;

                                glm::vec3 worldAxis(0.0f);
                                if (hitAxis == 0) worldAxis = glm::vec3(1.0f, 0.0f, 0.0f);
                                else if (hitAxis == 1) worldAxis = glm::vec3(0.0f, 1.0f, 0.0f);
                                else if (hitAxis == 2) worldAxis = glm::vec3(0.0f, 0.0f, 1.0f);

                                auto& mesh = sceneMeshes[m_selectedMeshIndex];
                                glm::vec3 gizmoCenter = glm::vec3(mesh->getModelMatrix() * glm::vec4(mesh->getGeometricCenter(), 1.0f));
                                m_gizmoPlaneOrigin = gizmoCenter;

                                glm::mat4 invView = glm::inverse(m_camera.getViewMatrix());
                                glm::vec3 camDir = -glm::vec3(invView[2]);
                                glm::vec3 camRight = glm::vec3(invView[0]);
                                glm::vec3 camUp = glm::vec3(invView[1]);

                                glm::vec3 N;
                                if (std::abs(glm::dot(camDir, worldAxis)) > 0.95f) {
                                    N = (std::abs(worldAxis.x) > 0.5f) ? camUp : camRight;
                                } else {
                                    N = glm::cross(worldAxis, glm::cross(camDir, worldAxis));
                                }
                                m_gizmoPlaneNormal = glm::normalize(N);

                                Ray ray = screenPointToRay(mousePos);
                                if (intersectRayPlane(ray.origin, ray.direction, m_gizmoPlaneOrigin, m_gizmoPlaneNormal, m_lastRayIntersection)) {
                                    m_isDraggingGizmo = true;
                                    m_gizmoLastIntersection = m_lastRayIntersection;
                                } else {
                                    m_isDraggingGizmo = false;
                                }

                                std::cout << "started dragging gizmo on axis: " << hitAxis << "\n";
                            }
                        }

                        m_viewportFocused = clickInsideViewport;
                    }
                }

                // mouse release handling
                if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        m_rightClickDragging = false;
                        m_cursorLocked = false;
                        SDL_SetWindowRelativeMouseMode(m_window, false);
                    } else if (event.button.button == SDL_BUTTON_LEFT) {
                        if (m_isDraggingGizmo) {
                            m_isDraggingGizmo = false;
                            m_activeGizmoAxis = -1;
                        }
                    }
                }

                // mouse motion handling
                if (event.type == SDL_EVENT_MOUSE_MOTION) {
                    if (m_isDraggingGizmo) {
                        auto& sceneMeshes = static_cast<VulkanRenderer*>(m_renderer.get())->getSceneMeshes();
                        if (!sceneMeshes.empty() && m_selectedMeshIndex < sceneMeshes.size() && sceneMeshes[m_selectedMeshIndex]) {
                            glm::vec2 currentMousePos(event.motion.x, event.motion.y);

                            glm::vec3 worldAxis(0.0f);
                            if (m_activeGizmoAxis == 0) worldAxis = glm::vec3(1.0f, 0.0f, 0.0f);
                            else if (m_activeGizmoAxis == 1) worldAxis = glm::vec3(0.0f, 1.0f, 0.0f);
                            else if (m_activeGizmoAxis == 2) worldAxis = glm::vec3(0.0f, 0.0f, 1.0f);

                            Ray ray = screenPointToRay(currentMousePos);
                            glm::vec3 currentIntersection;
                            if (intersectRayPlane(ray.origin, ray.direction, m_gizmoPlaneOrigin, m_gizmoPlaneNormal, currentIntersection)) {
                                glm::vec3 frameWorldDelta = currentIntersection - m_gizmoLastIntersection;
                                m_gizmoLastIntersection = currentIntersection;

                                float frameAxisDelta = glm::dot(frameWorldDelta, worldAxis);

                                glm::vec3 translationDelta = worldAxis * frameAxisDelta;
                                if (glm::length(translationDelta) > 0.0001f) {
                                    TranslateMeshCommand translateCmd(m_selectedMeshIndex, translationDelta);
                                    translateCmd.execute(m_commandContext);
                                }
                            }
                        }
                    } else if (m_cursorLocked) {
                        m_camera.processMouseMovement(event.motion.xrel, -event.motion.yrel);
                    }
                }

                if (event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                    int w = event.window.data1;
                    int h = event.window.data2;

                    m_width = w;
                    m_height = h;

                    m_renderer->onWindowResize(w, h);

                    if (m_uiManager) {
                        m_uiManager->setScreenSize(static_cast<float>(w), static_cast<float>(h));

                        auto rootElement = m_uiManager->getRootElement();
                        if (rootElement) {
                            rootElement->setSize(glm::vec2(static_cast<float>(w), static_cast<float>(h)));
                        }

                        m_uiManager->markDirty();
                    }
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

            glm::vec2 vpOffset = m_viewportPanel ? m_viewportPanel->getAbsolutePosition() : glm::vec2(0.0f);
            glm::vec2 vpSize = m_viewportPanel ? m_viewportPanel->getSize() : glm::vec2(static_cast<float>(m_width), static_cast<float>(m_height));

            m_renderer->drawFrame(m_camera.getViewMatrix(), vpOffset, vpSize);
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