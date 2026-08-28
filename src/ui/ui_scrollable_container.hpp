#pragma once

#include "ui_element.hpp"
#include <SDL3/SDL.h>
#include <algorithm>

namespace slate {

    class UIManager;

    class UIScrollableContainer : public UIElement {
    public:
        UIScrollableContainer(const std::string& name, glm::vec2 position, glm::vec2 size);

        float getScrollOffset() const { return m_scrollOffset; }
        void setUIManager(UIManager* manager) { m_uiManager = manager; }

        void onEvent(const SDL_Event& event) override;
        void generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) override;

    private:
        void updateLayout();

        float m_scrollOffset = 0.0f;
        UIManager* m_uiManager = nullptr;
    };

}