#pragma once

#include "ui_element.hpp"
#include <SDL3/SDL.h>
#include <algorithm>

namespace slate {

    class UIScrollableContainer : public UIElement {
    public:
        UIScrollableContainer(const std::string& name, glm::vec2 position, glm::vec2 size);

        void setTotalContentHeight(float height) { m_totalContentHeight = height; }
        float getScrollOffset() const { return m_scrollOffset; }

        void onEvent(const SDL_Event& event) override;
        void generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) override;

    private:
        float m_scrollOffset = 0.0f;
        float m_totalContentHeight = 0.0f;
    };

}