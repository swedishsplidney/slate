#include "ui_scrollable_container.hpp"
#include <iostream>

namespace slate {

    UIScrollableContainer::UIScrollableContainer(const std::string& name, glm::vec2 position, glm::vec2 size)
        : UIElement(name, position, size) {
    }

    void UIScrollableContainer::onEvent(const SDL_Event& event) {
        UIElement::onEvent(event);

        if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            float mx = 0.0f, my = 0.0f;
            SDL_GetMouseState(&mx, &my);
            glm::vec2 absPos = getAbsolutePosition();
            
            if (mx >= absPos.x && mx <= absPos.x + m_size.x &&
                my >= absPos.y && my <= absPos.y + m_size.y) {

                m_scrollOffset -= event.wheel.y * 25.0f;

                float maxScroll = std::max(0.0f, m_totalContentHeight - m_size.y);
                m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxScroll);
                }
        }
    }

    void UIScrollableContainer::generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) {
        if (!m_visible) return;

        float currentY = 0.0f - m_scrollOffset;
        for (auto& child : m_children) {
            glm::vec2 originalPos = child->getPosition();

            child->setPosition(glm::vec2(originalPos.x, currentY));

            child->generateGeometry(vertices, indices);

            currentY += child->getSize().y;

            child->setPosition(originalPos);
        }
    }

}