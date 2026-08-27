#include "ui_hierarchy_panel.hpp"
#include <iostream>
#include <SDL3/SDL.h>

namespace slate {
    UIHierarchyPanel::UIHierarchyPanel(const std::string& name, glm::vec2 position, glm::vec2 size)
        : UIElement(name, position, size) {
        setDrawsBackground(true);
        setColor(glm::vec4(0.13f, 0.14f, 0.18f, 1.0f));
    }

    void UIHierarchyPanel::setSize(glm::vec2 size) {
        UIElement::setSize(size);
    }

    void UIHierarchyPanel::buildDefaultLayout() {

    }

    void UIHierarchyPanel::setSceneItems(const std::vector<std::string>& itemNames) {
        m_itemNames = itemNames;
    }

    void UIHierarchyPanel::onEvent(const SDL_Event& event) {
        UIElement::onEvent(event);
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
            glm::vec2 mousePos(event.button.x, event.button.y);
            glm::vec2 absPos = getAbsolutePosition();

            if (mousePos.x >= absPos.x && mousePos.x <= absPos.x + m_size.x &&
                mousePos.y >= absPos.y + 28.0f && mousePos.y <= absPos.y + m_size.y) {

                float relativeY = mousePos.y - (absPos.y + 28.0f);
                int clickedIndex = static_cast<int>(relativeY / 24.0f);

                if (clickedIndex >= 0 && clickedIndex < static_cast<int>(m_itemNames.size())) {
                    m_selectedIndex = clickedIndex;
                    if (m_onItemSelected) {
                        m_onItemSelected(m_selectedIndex);
                    }
                }
                }
        }
    }

    void UIHierarchyPanel::generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) {
        if (!m_visible) return;

        glm::vec2 absPos = getAbsolutePosition();
        float panelWidth = m_size.x > 0 ? m_size.x : 300.0f;
        float panelHeight = m_size.y > 0 ? m_size.y : 300.0f;

        // bg
        glm::vec4 panelBgColor(0.004f, 0.0045f, 0.0055f, 1.0f);
        uint16_t panelIdx = static_cast<uint16_t>(vertices.size());
        vertices.push_back(UIVertex{.pos = absPos, .color = panelBgColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + panelWidth, absPos.y), .color = panelBgColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + panelWidth, absPos.y + panelHeight), .color = panelBgColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x, absPos.y + panelHeight), .color = panelBgColor, .uv = glm::vec2(-1.0f)});
        indices.push_back(panelIdx + 0); indices.push_back(panelIdx + 1); indices.push_back(panelIdx + 2);
        indices.push_back(panelIdx + 0); indices.push_back(panelIdx + 2); indices.push_back(panelIdx + 3);

        // title bar
        glm::vec4 headerColor(0.016f, 0.018f, 0.023f, 1.0f);
        uint16_t hIdx = static_cast<uint16_t>(vertices.size());
        vertices.push_back(UIVertex{.pos = absPos, .color = headerColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + panelWidth, absPos.y), .color = headerColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + panelWidth, absPos.y + 28.0f), .color = headerColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x, absPos.y + 28.0f), .color = headerColor, .uv = glm::vec2(-1.0f)});
        indices.push_back(hIdx + 0); indices.push_back(hIdx + 1); indices.push_back(hIdx + 2);
        indices.push_back(hIdx + 0); indices.push_back(hIdx + 2); indices.push_back(hIdx + 3);

        if (!m_fontLoader) return;

        glm::vec4 textColor(0.85f, 0.85f, 0.90f, 1.0f);
        m_fontLoader->generateTextGeometry("Hierarchy", glm::vec2(absPos.x + 10.0f, absPos.y + 18.0f), textColor, vertices, indices);

        float startY = absPos.y + 36.0f;
        for (size_t i = 0; i < m_itemNames.size(); ++i) {
            float itemY = startY + (i * 24.0f);
            if (static_cast<int>(i) == m_selectedIndex) {
                glm::vec4 selColor(0.20f, 0.25f, 0.40f, 1.0f);
                uint16_t sIdx = static_cast<uint16_t>(vertices.size());
                vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + 4.0f, itemY), .color = selColor, .uv = glm::vec2(-1.0f)});
                vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + panelWidth - 4.0f, itemY), .color = selColor, .uv = glm::vec2(-1.0f)});
                vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + panelWidth - 4.0f, itemY + 22.0f), .color = selColor, .uv = glm::vec2(-1.0f)});
                vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + 4.0f, itemY + 22.0f), .color = selColor, .uv = glm::vec2(-1.0f)});
                indices.push_back(sIdx + 0); indices.push_back(sIdx + 1); indices.push_back(sIdx + 2);
                indices.push_back(sIdx + 0); indices.push_back(sIdx + 2); indices.push_back(sIdx + 3);
            }
            m_fontLoader->generateTextGeometry(m_itemNames[i], glm::vec2(absPos.x + 12.0f, itemY + 16.0f), textColor, vertices, indices);
        }
    }
}