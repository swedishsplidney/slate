#include "engine.hpp"
#include "renderer/vulkan/vulkan_renderer.hpp"
#include "resources/mesh_loader.hpp"
#include "ui/ui_manager.hpp"
#include "ui/ui_dockspace.hpp"
#include "ui/ui_button.hpp"
#include "core/commands/file_commands.hpp"
#include "core/commands/editor_commands.hpp"
#include "core/commands/translate_commands.hpp"
#include "core/commands/rotate_commands.hpp"
#include "core/commands/material_commands.hpp"
#include "core/commands/scale_commands.hpp"
#include "core/commands/set_position_commands.hpp"
#include "core/commands/set_rotation_commands.hpp"
#include "ui/ui_menu_bar.hpp"

#include <stdexcept>
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <algorithm>

namespace slate {

    Ray Engine::screenPointToRay(glm::vec2 mousePos) {
        glm::vec2 vpOffset = m_viewportPanel ? m_viewportPanel->getAbsolutePosition() : glm::vec2(0.0f);
        glm::vec2 vpSize = m_viewportPanel ? m_viewportPanel->getSize() : glm::vec2(static_cast<float>(m_width), static_cast<float>(m_height));

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
        auto leftPanel = std::make_shared<UIElement>("LeftPanel", glm::vec2(0.0f), glm::vec2(0.0f));
        leftPanel->setDrawsBackground(true);
        leftPanel->setColor(glm::vec4(0.004f, 0.005f, 0.008f, 0.1f));
        mainDockSpace->addDockedChild(leftPanel, DockSlot::LeftSide, 0.0f);

        // right
        m_inspectorPanel = std::make_shared<UIInspectorPanel>(
            "InspectorPanel",
            glm::vec2(0.0f),
            glm::vec2(0.0f)
        );

        m_inspectorPanel->setFontLoader(fontLoader);
        m_inspectorPanel->buildDefaultLayout();
        m_inspectorPanel->setDrawsBackground(true);
        m_inspectorPanel->setColor(glm::vec4(0.008f, 0.015f, 0.016f, 0.1f));

        m_inspectorPanel->setOnPositionChanged([this](float x, float y, float z) {
            m_commandRegistry->execute("editor.set_position", m_commandContext, {
                std::to_string(x), std::to_string(y), std::to_string(z)
            });
        });

        m_inspectorPanel->setOnRotationChanged([this](float x, float y, float z) {
            glm::quat q = glm::quat(glm::radians(glm::vec3(x, y, z)));
            m_commandRegistry->execute("editor.set_rotation", m_commandContext, {
                std::to_string(q.w), std::to_string(q.x), std::to_string(q.y), std::to_string(q.z)
            });
        });

        m_inspectorPanel->setOnScaleChanged([this](float x, float y, float z) {
            m_commandRegistry->execute("editor.set_scale", m_commandContext, {
                std::to_string(x), std::to_string(y), std::to_string(z)
            });
        });

        mainDockSpace->addDockedChild(m_inspectorPanel, DockSlot::RightSide, 300.0f);

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

        m_commandRegistry->registerCommand("editor.redo", [this](const CommandRegistry::CommandArgs&) {
            m_commandRegistry->getHistory().redo(m_commandContext);
            return nullptr;
        });

        m_commandRegistry->registerCommand("editor.translate_mesh", [this](const CommandRegistry::CommandArgs& args) {
            glm::vec3 delta(0.0f, 0.5f, 0.0f);
            return std::make_unique<TranslateMeshCommand>(m_selectedMeshIndex, delta);
        });

        m_commandRegistry->registerCommand("editor.rotate_mesh", [this](const CommandRegistry::CommandArgs& args) {
            glm::quat deltaRot = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            return std::make_unique<RotateMeshCommand>(m_selectedMeshIndex, deltaRot);
        });

        m_commandRegistry->registerCommand("editor.set_position", [this](const CommandRegistry::CommandArgs& args) -> std::unique_ptr<ICommand> {
            glm::vec3 pos(0.0f);
            if (args.size() >= 3) {
                pos.x = std::stof(args[0]);
                pos.y = std::stof(args[1]);
                pos.z = std::stof(args[2]);
            }
            return std::make_unique<SetPositionCommand>(m_selectedMeshIndex, pos);
        });

        m_commandRegistry->registerCommand("editor.set_scale", [this](const CommandRegistry::CommandArgs& args) -> std::unique_ptr<ICommand> {
            glm::vec3 scale(1.0f);
            if (args.size() >= 3) {
                scale.x = std::stof(args[0]);
                scale.y = std::stof(args[1]);
                scale.z = std::stof(args[2]);
            }
            return std::make_unique<SetScaleCommand>(m_selectedMeshIndex, scale);
        });

        m_commandRegistry->registerCommand("editor.set_rotation", [this](const CommandRegistry::CommandArgs& args) -> std::unique_ptr<ICommand> {
            glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
            if (args.size() >= 4) {
                rot.w = std::stof(args[0]);
                rot.x = std::stof(args[1]);
                rot.y = std::stof(args[2]);
                rot.z = std::stof(args[3]);
            }
            return std::make_unique<SetRotationCommand>(m_selectedMeshIndex, rot);
        });

        m_commandRegistry->registerCommand("material.update_float", [this](const CommandRegistry::CommandArgs& args) -> std::unique_ptr<ICommand> {
            if (args.size() < 3) return nullptr;
            uint32_t matId = static_cast<uint32_t>(std::stoul(args[0]));
            MaterialPropertyType prop = static_cast<MaterialPropertyType>(std::stoi(args[1]));
            float val = std::stof(args[2]);
            return std::make_unique<UpdateMaterialCommand>(m_selectedMeshIndex, matId, prop, val);
        });

        m_commandRegistry->registerCommand("material.update_vec4", [this](const CommandRegistry::CommandArgs& args) -> std::unique_ptr<ICommand> {
            if (args.size() < 6) return nullptr;
            uint32_t matId = static_cast<uint32_t>(std::stoul(args[0]));
            MaterialPropertyType prop = static_cast<MaterialPropertyType>(std::stoi(args[1]));
            glm::vec4 val(std::stof(args[2]), std::stof(args[3]), std::stof(args[4]), std::stof(args[5]));
            return std::make_unique<UpdateMaterialCommand>(m_selectedMeshIndex, matId, prop, val);
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
        glm::vec2 vpOffset = m_viewportPanel ? m_viewportPanel->getAbsolutePosition() : glm::vec2(0.0f);
        glm::vec2 vpSize = m_viewportPanel ? m_viewportPanel->getSize() : glm::vec2(static_cast<float>(m_width), static_cast<float>(m_height));
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

        auto vkRenderer = static_cast<VulkanRenderer*>(m_renderer.get());
        auto gizmoMode = vkRenderer->getGizmoMode();

        auto& mesh = sceneMeshes[m_selectedMeshIndex];
        glm::vec3 gizmoCenter = glm::vec3(mesh->getModelMatrix() * glm::vec4(mesh->getGeometricCenter(), 1.0f));

        float dist = glm::distance(m_camera.getPosition(), gizmoCenter);
        float t = dist / (dist + 8.0f);
        float gizmoScale = glm::mix(0.05f, 1.0f, t);

        glm::vec2 vpOffset = m_viewportPanel ? m_viewportPanel->getAbsolutePosition() : glm::vec2(0.0f);
        glm::vec2 vpSize = m_viewportPanel ? m_viewportPanel->getSize() : glm::vec2(static_cast<float>(m_width), static_cast<float>(m_height));
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

        glm::mat4 modelMat = mesh->getModelMatrix();
        glm::vec3 colX = glm::normalize(glm::vec3(modelMat[0]));
        glm::vec3 colY = glm::normalize(glm::vec3(modelMat[1]));
        glm::vec3 colZ = glm::normalize(glm::vec3(modelMat[2]));
        glm::mat3 meshRot(colX, colY, colZ);

        glm::mat3 assetCorrection = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::mat3 finalRot = meshRot * assetCorrection;

        glm::vec3 axisX = finalRot * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 axisY = finalRot * glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 axisZ = finalRot * glm::vec3(0.0f, 0.0f, 1.0f);

        float distX = 99999.0f, distY = 99999.0f, distZ = 99999.0f;
        float threshold = 12.0f;

        if (gizmoMode == VulkanRenderer::GizmoMode::Translate) {
            glm::vec2 screenPosX, screenPosY, screenPosZ;
            const float minScreenLengthForHit = 5.0f;

            if (getValidScreenPos(gizmoCenter + axisX * gizmoScale, screenPosX)) {
                if (glm::length(screenPosX - screenOrigin) >= minScreenLengthForHit) {
                    distX = distToSegment(mousePos, screenOrigin, screenPosX);
                }
            }

            if (getValidScreenPos(gizmoCenter + axisY * gizmoScale, screenPosY)) {
                if (glm::length(screenPosY - screenOrigin) >= minScreenLengthForHit) {
                    distY = distToSegment(mousePos, screenOrigin, screenPosY);
                }
            }

            if (getValidScreenPos(gizmoCenter + axisZ * gizmoScale, screenPosZ)) {
                if (glm::length(screenPosZ - screenOrigin) >= minScreenLengthForHit) {
                    distZ = distToSegment(mousePos, screenOrigin, screenPosZ);
                }
            }
        }
        else if (gizmoMode == VulkanRenderer::GizmoMode::Rotate) {
            auto checkCircleHit = [&](const glm::vec3& uAxis, const glm::vec3& vAxis) {
                float minCircleDist = 99999.0f;
                int segments = 32;
                glm::vec2 prevScreenPos;
                bool prevValid = false;

                for (int i = 0; i <= segments; ++i) {
                    float angle = (static_cast<float>(i) / segments) * 2.0f * glm::pi<float>();
                    glm::vec3 worldPos = gizmoCenter + gizmoScale * (std::cos(angle) * uAxis + std::sin(angle) * vAxis);
                    glm::vec2 screenPos;
                    if (getValidScreenPos(worldPos, screenPos)) {
                        if (prevValid) {
                            float d = distToSegment(mousePos, prevScreenPos, screenPos);
                            minCircleDist = std::min(minCircleDist, d);
                        }
                        prevScreenPos = screenPos;
                        prevValid = true;
                    } else {
                        prevValid = false;
                    }
                }
                return minCircleDist;
            };

            distX = checkCircleHit(axisY, axisZ);
            distY = checkCircleHit(axisX, axisZ);
            distZ = checkCircleHit(axisX, axisY);
        }

        float minDist = std::min({distX, distY, distZ});

        if (minDist <= threshold) {
            if (minDist == distX) return 0;
            if (minDist == distY) return 1;
            if (minDist == distZ) return 2;
        }

        return -1;
    }

    void Engine::setSelectedMeshIndex(int index) {
        m_selectedMeshIndex = index;

        if (m_inspectorPanel) {
            auto& sceneMeshes = static_cast<VulkanRenderer*>(m_renderer.get())->getSceneMeshes();
            if (m_selectedMeshIndex >= 0 && m_selectedMeshIndex < sceneMeshes.size() && sceneMeshes[m_selectedMeshIndex]) {
                auto& mesh = sceneMeshes[m_selectedMeshIndex];
                m_inspectorPanel->setTargetObject(mesh->getName());

                glm::mat4 modelMat = sceneMeshes[m_selectedMeshIndex]->getModelMatrix();
                glm::vec3 currentPos = glm::vec3(modelMat[3]);
                glm::vec3 currentScale(glm::length(modelMat[0]), glm::length(modelMat[1]), glm::length(modelMat[2]));

                glm::mat3 rotMat(
                    modelMat[0] / (currentScale.x != 0.0f ? currentScale.x : 1.0f),
                    modelMat[1] / (currentScale.y != 0.0f ? currentScale.y : 1.0f),
                    modelMat[2] / (currentScale.z != 0.0f ? currentScale.z : 1.0f)
                );
                glm::vec3 currentRot = glm::degrees(glm::eulerAngles(glm::quat_cast(rotMat)));

                m_inspectorPanel->setPositionValues(currentPos);
                m_inspectorPanel->setRotationValues(currentRot);
                m_inspectorPanel->setScaleValues(currentScale);
            } else {
                m_inspectorPanel->setTargetObject("None");
                m_inspectorPanel->setPositionValues(glm::vec3(0.0f));
                m_inspectorPanel->setRotationValues(glm::vec3(0.0f));
                m_inspectorPanel->setScaleValues(glm::vec3(1.0f));
            }
        }

        if (m_uiManager) {
            m_uiManager->markDirty();
        }
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
                    float mx = static_cast<float>(event.button.x);
                    float my = static_cast<float>(event.button.y);
                    mousePos = glm::vec2(mx, my);
                    clickInsideViewport = isPointInElement(mousePos, m_viewportPanel);

                    if (clickInsideViewport && (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT)) {
                        m_viewportFocused = true;
                    } else {
                        m_viewportFocused = false;
                    }

                    if (event.button.button == SDL_BUTTON_RIGHT && clickInsideViewport) {
                        m_rightClickDragging = true;
                        m_cursorLocked = true;
                        SDL_SetWindowRelativeMouseMode(m_window, true);
                    } else if (event.button.button == SDL_BUTTON_LEFT && clickInsideViewport) {
                        int hitAxis = checkGizmoHit(mousePos);

                        if (hitAxis != -1) {
                            // gizmo
                            auto& sceneMeshes = static_cast<VulkanRenderer*>(m_renderer.get())->getSceneMeshes();
                            if (!sceneMeshes.empty() && m_selectedMeshIndex < sceneMeshes.size() && sceneMeshes[m_selectedMeshIndex]) {
                                m_activeGizmoAxis = hitAxis;
                                m_gizmoDragStartPos = mousePos;

                                auto& mesh = sceneMeshes[m_selectedMeshIndex];
                                glm::mat4 modelMat = mesh->getModelMatrix();
                                glm::vec3 colX = glm::normalize(glm::vec3(modelMat[0]));
                                glm::vec3 colY = glm::normalize(glm::vec3(modelMat[1]));
                                glm::vec3 colZ = glm::normalize(glm::vec3(modelMat[2]));
                                glm::mat3 meshRot(colX, colY, colZ);

                                glm::mat3 assetCorrection = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
                                glm::mat3 finalRot = meshRot * assetCorrection;

                                glm::vec3 axisX = finalRot * glm::vec3(1.0f, 0.0f, 0.0f);
                                glm::vec3 axisY = finalRot * glm::vec3(0.0f, 1.0f, 0.0f);
                                glm::vec3 axisZ = finalRot * glm::vec3(0.0f, 0.0f, 1.0f);

                                glm::vec3 worldAxis(0.0f);
                                if (hitAxis == 0) worldAxis = axisX;
                                else if (hitAxis == 1) worldAxis = axisY;
                                else if (hitAxis == 2) worldAxis = axisZ;

                                m_dragWorldAxis = worldAxis;

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
                            }
                        }
                        else {
                            auto& sceneMeshes = static_cast<VulkanRenderer*>(m_renderer.get())->getSceneMeshes();
                            int closestMeshIndex = -1;
                            float closestDist = std::numeric_limits<float>::max();

                            Ray ray = screenPointToRay(mousePos);

                            for (size_t i = 0; i < sceneMeshes.size(); ++i) {
                                if (!sceneMeshes[i]) continue;

                                glm::mat4 invModel = glm::inverse(sceneMeshes[i]->getModelMatrix());
                                glm::vec3 localOrigin = glm::vec3(invModel * glm::vec4(ray.origin, 1.0f));
                                glm::vec3 localDir = glm::normalize(glm::vec3(invModel * glm::vec4(ray.direction, 0.0f)));

                                const auto& vertices = sceneMeshes[i]->getVertices();
                                const auto& indices = sceneMeshes[i]->getIndices();

                                for (size_t j = 0; j < indices.size(); j += 3) {
                                    glm::vec3 v0 = vertices[indices[j]].pos;
                                    glm::vec3 v1 = vertices[indices[j + 1]].pos;
                                    glm::vec3 v2 = vertices[indices[j + 2]].pos;

                                    glm::vec3 edge1 = v1 - v0;
                                    glm::vec3 edge2 = v2 - v0;
                                    glm::vec3 h = glm::cross(localDir, edge2);
                                    float a = glm::dot(edge1, h);

                                    if (a > -0.00001f && a < 0.00001f) continue;

                                    float f = 1.0f / a;
                                    glm::vec3 s = localOrigin - v0;
                                    float u = f * glm::dot(s, h);
                                    if (u < 0.0f || u > 1.0f) continue;

                                    glm::vec3 q = glm::cross(s, edge1);
                                    float v = f * glm::dot(localDir, q);
                                    if (v < 0.0f || u + v > 1.0f) continue;

                                    float t = f * glm::dot(edge2, q);
                                    if (t > 0.0001f && t < closestDist) {
                                        closestDist = t;
                                        closestMeshIndex = static_cast<int>(i);
                                    }
                                }
                            }

                            setSelectedMeshIndex(closestMeshIndex);

                            if (m_inspectorPanel) {
                                auto& sceneMeshes = static_cast<VulkanRenderer*>(m_renderer.get())->getSceneMeshes();
                                if (m_selectedMeshIndex >= 0 && m_selectedMeshIndex < sceneMeshes.size() && sceneMeshes[m_selectedMeshIndex]) {
                                    m_inspectorPanel->setTargetObject(sceneMeshes[m_selectedMeshIndex]->getName());

                                    if (!m_isDraggingGizmo) {
                                        glm::vec3 currentPos = glm::vec3(sceneMeshes[m_selectedMeshIndex]->getModelMatrix()[3]);
                                        m_inspectorPanel->setPositionValues(currentPos);
                                    }
                                } else {
                                    m_inspectorPanel->setTargetObject("None");
                                    m_inspectorPanel->setPositionValues(glm::vec3(0.0f));
                                }
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
                        auto vkRenderer = static_cast<VulkanRenderer*>(m_renderer.get());
                        auto& sceneMeshes = vkRenderer->getSceneMeshes();

                        if (!sceneMeshes.empty() && m_selectedMeshIndex < sceneMeshes.size() && sceneMeshes[m_selectedMeshIndex]) {
                            glm::vec2 currentMousePos(event.motion.x, event.motion.y);

                            if (vkRenderer->getGizmoMode() == VulkanRenderer::GizmoMode::Translate) {
                                Ray ray = screenPointToRay(currentMousePos);
                                glm::vec3 currentIntersection;
                                if (intersectRayPlane(ray.origin, ray.direction, m_gizmoPlaneOrigin, m_gizmoPlaneNormal, currentIntersection)) {
                                    glm::vec3 frameWorldDelta = currentIntersection - m_gizmoLastIntersection;
                                    m_gizmoLastIntersection = currentIntersection;

                                    float frameAxisDelta = glm::dot(frameWorldDelta, m_dragWorldAxis);
                                    glm::vec3 translationDelta = m_dragWorldAxis * frameAxisDelta;

                                    if (glm::length(translationDelta) > 0.0001f) {
                                        TranslateMeshCommand translateCmd(m_selectedMeshIndex, translationDelta);
                                        translateCmd.execute(m_commandContext);

                                        if (m_inspectorPanel && sceneMeshes[m_selectedMeshIndex]) {
                                            glm::vec3 currentPos = glm::vec3(sceneMeshes[m_selectedMeshIndex]->getModelMatrix()[3]);
                                            m_inspectorPanel->setPositionValues(currentPos);
                                        }
                                    }
                                }
                            }
                            else if (vkRenderer->getGizmoMode() == VulkanRenderer::GizmoMode::Rotate) {
                                float sensitivity = 0.01f;
                                float deltaAngle = (event.motion.xrel - event.motion.yrel) * sensitivity;

                                if (std::abs(deltaAngle) > 0.0001f) {
                                    glm::quat deltaRot = glm::angleAxis(deltaAngle, m_dragWorldAxis);
                                    RotateMeshCommand rotateCmd(m_selectedMeshIndex, deltaRot);
                                    rotateCmd.execute(m_commandContext);
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

            if (m_viewportPanel) {
                glm::vec2 vpOffset = m_viewportPanel->getAbsolutePosition();
                glm::vec2 vpSize = m_viewportPanel->getSize();

                if (vpSize.x > 0.0f && vpSize.y > 0.0f) {
                    auto vkRenderer = static_cast<VulkanRenderer*>(m_renderer.get());
                    vkRenderer->setViewportBounds(vpOffset, vpSize);
                }
            }

            if (m_uiManager && m_uiManager->isDirty()) {
                m_uiManager->rebuildGeometry();

                auto& vertices = m_uiManager->getVertexBuffer();
                auto& indices = m_uiManager->getIndexBuffer();

                static_cast<VulkanRenderer*>(m_renderer.get())->updateUIGeometryBuffers(vertices, indices);
            }

            auto vkRenderer = static_cast<VulkanRenderer*>(m_renderer.get());
            vkRenderer->setSelectedMeshIndex(m_selectedMeshIndex);

            glm::vec2 vpOffset(0.0f, 0.0f);
            glm::vec2 vpSize(static_cast<float>(m_width), static_cast<float>(m_height));

            if (m_viewportPanel) {
                vpOffset = m_viewportPanel->getAbsolutePosition();
                vpSize = m_viewportPanel->getSize();
            }

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