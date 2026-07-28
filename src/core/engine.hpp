#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include "ui/ui_element.hpp"
#include "ui/ui_manager.hpp"

#include "camera.hpp"
#include "renderer/renderer.hpp"

namespace slate {

    class Engine {
    public:
        Engine();
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        void run();

    private:
        void initWindow();
        void mainLoop();
        void cleanup();

        bool isPointInElement(glm::vec2 point, const std::shared_ptr<UIElement>& element);

        SDL_Window* m_window{nullptr};
        const int m_width{1280};
        const int m_height{720};

        std::unique_ptr<Renderer> m_renderer{nullptr};

        Camera m_camera{glm::vec3(0.0f, 0.0f, 4.0f)};
        uint64_t m_lastTime{0};
        bool m_cursorLocked{false};
        
        std::shared_ptr<UIElement> m_viewportPanel{nullptr};
        bool m_viewportFocused{false};
        bool m_rightClickDragging{false};

        std::unique_ptr<UIManager> m_uiManager;
    };

}