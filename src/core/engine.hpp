#pragma once

#include <memory>
#include <SDL3/SDL.h>
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

        SDL_Window* m_window{nullptr};
        const int m_width{800};
        const int m_height{600};

        std::unique_ptr<Renderer> m_renderer{nullptr};
    };

}