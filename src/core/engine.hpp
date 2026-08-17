#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include "ui/ui_element.hpp"
#include "ui/ui_manager.hpp"

#include "camera.hpp"
#include "renderer/renderer.hpp"
#include "core/commands/command_registry.hpp"

namespace slate {

    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    class Engine {
    public:
        Engine();
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        Ray screenPointToRay(glm::vec2 mousePos);

        void run();

    private:
        void initWindow();
        void registerDefaultCommands();
        void mainLoop();
        void cleanup();

        bool isPointInElement(glm::vec2 point, const std::shared_ptr<UIElement>& element);

        SDL_Window* m_window{nullptr};
        int m_width{1280};
        int m_height{720};

        std::unique_ptr<Renderer> m_renderer{nullptr};

        Camera m_camera{glm::vec3(0.0f, 0.0f, 4.0f)};
        uint64_t m_lastTime{0};
        bool m_cursorLocked{false};
        
        std::shared_ptr<UIElement> m_viewportPanel{nullptr};
        bool m_viewportFocused{false};
        bool m_rightClickDragging{false};

        std::unique_ptr<UIManager> m_uiManager{nullptr};
        std::unique_ptr<CommandRegistry> m_commandRegistry{nullptr};
        CommandContext m_commandContext{};

        size_t m_selectedMeshIndex = 0;

        bool m_isDraggingGizmo = {false};
        int m_activeGizmoAxis = {-1};
        glm::vec2 m_gizmoDragStartPos{0.0f};
        int checkGizmoHit(const glm::vec2& mousePos);
        glm::vec2 worldToScreen(const glm::vec3& worldPos);

        glm::vec3 m_gizmoPlaneOrigin;
        glm::vec3 m_gizmoPlaneNormal;
        glm::vec3 m_lastRayIntersection;

        glm::vec3 m_gizmoDragStartIntersection{0.0f};
        glm::vec3 m_gizmoLastIntersection{0.0f};

        glm::vec3 m_dragWorldAxis{0.0f};
    };

}