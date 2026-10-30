#pragma once

#include "ui/ui_vertex.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>

namespace slate {

    inline void pushQuadColors(std::vector<UIVertex>& v, std::vector<UIIndex>& idx,
                               glm::vec2 pos, glm::vec2 size,
                               const glm::vec4& cTL, const glm::vec4& cTR,
                               const glm::vec4& cBR, const glm::vec4& cBL,
                               float radius = 0.0f) {
        (void)radius;

        const UIIndex base = static_cast<UIIndex>(v.size());
        const glm::vec2 untextured(-1.0f);

        v.push_back(UIVertex{.pos = pos,                              .color = cTL, .uv = untextured});
        v.push_back(UIVertex{.pos = pos + glm::vec2(size.x, 0.0f),    .color = cTR, .uv = untextured});
        v.push_back(UIVertex{.pos = pos + size,                       .color = cBR, .uv = untextured});
        v.push_back(UIVertex{.pos = pos + glm::vec2(0.0f, size.y),    .color = cBL, .uv = untextured});

        idx.push_back(base + 0); idx.push_back(base + 1); idx.push_back(base + 2);
        idx.push_back(base + 0); idx.push_back(base + 2); idx.push_back(base + 3);
    }

    inline void pushQuad(std::vector<UIVertex>& v, std::vector<UIIndex>& idx,
                         glm::vec2 pos, glm::vec2 size, const glm::vec4& color,
                         float radius = 0.0f) {
        pushQuadColors(v, idx, pos, size, color, color, color, color, radius);
    }

}
