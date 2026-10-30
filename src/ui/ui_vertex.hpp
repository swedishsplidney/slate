#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <cstdint>

using UIIndex = uint16_t;

namespace slate {

    struct UIVertex {
        glm::vec2 pos;
        glm::vec4 color;
        glm::vec2 uv;
    };

}