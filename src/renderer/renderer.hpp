#pragma once

namespace slate {

    class Renderer {
    public:
        virtual ~Renderer() = default;

        virtual void init() = 0;
        virtual void drawFrame() = 0;
        virtual void cleanup() = 0;
        virtual void onWindowResize(int width, int height) {}
    };

}