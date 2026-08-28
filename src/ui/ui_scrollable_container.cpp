#include "ui_scrollable_container.hpp"
#include <algorithm>
#include <iostream>
#include "ui/ui_manager.hpp"

namespace slate {

    UIScrollableContainer::UIScrollableContainer(const std::string& name, glm::vec2 position, glm::vec2 size)
        : UIElement(name, position, size) {
    }

    void UIScrollableContainer::updateLayout() {
        float totalContentHeight = 0.0f;
        for (auto& child : m_children) {
            totalContentHeight += child->getSize().y;
        }

        float maxScroll = std::max(0.0f, totalContentHeight - m_size.y);
        m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxScroll);

        float currentY = 0.0f - m_scrollOffset;
        for (auto& child : m_children) {
            child->setPosition(glm::vec2(0.0f, currentY));
            child->setSize(glm::vec2(m_size.x, child->getSize().y));
            currentY += child->getSize().y;
        }
    }

    void UIScrollableContainer::onEvent(const SDL_Event& event) {
        updateLayout();

        UIElement::onEvent(event);

        if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            float mx = 0.0f, my = 0.0f;
            SDL_GetMouseState(&mx, &my);
            glm::vec2 absPos = getAbsolutePosition();

            if (mx >= absPos.x && mx <= absPos.x + m_size.x &&
                my >= absPos.y && my <= absPos.y + m_size.y) {

                float oldOffset = m_scrollOffset;
                m_scrollOffset -= event.wheel.y * 25.0f;
                updateLayout();

                if (m_scrollOffset != oldOffset) {
                    if (m_uiManager) {
                        m_uiManager->markDirty();
                    }
                }
                }
        }
    }

    void UIScrollableContainer::generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) {
        if (!m_visible) return;

        updateLayout();

        for (auto& child : m_children) {
            child->generateGeometry(vertices, indices);
        }
    }

}