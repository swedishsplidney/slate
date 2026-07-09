#pragma once

#include <glm/glm.hpp>

namespace slate {

    class Renderer {
    public:
        virtual ~Renderer() = default;

        virtual void init() = 0;
        virtual void drawFrame(const glm::mat4& viewMatrix) = 0;
        virtual void cleanup() = 0;
        virtual void onWindowResize(int width, int height) {}
    };

}