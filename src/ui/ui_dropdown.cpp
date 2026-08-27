#include "ui_dropdown.hpp"
#include <iostream>

namespace slate {

    UIDropdown::UIDropdown(const std::string& name, glm::vec2 position, glm::vec2 size, const std::string& title, bool defualtExpanded)
        : UIElement(name, position, size), m_title(title), m_isExpanded(defualtExpanded) {
        setDrawsBackground(false);

        m_contentContainer = std::make_shared<UIElement>(name + "_Content", glm::vec2(0.0f, m_headerHeight), glm::vec2(size.x, size.y - m_headerHeight));
        m_contentContainer->setDrawsBackground(false);
        m_contentContainer->setVisible(m_isExpanded);
    }

    void UIDropdown::setExpanded(bool expanded) {
        if (m_isExpanded != expanded) {
            m_isExpanded = expanded;
            if (m_contentContainer) {
                m_contentContainer->setVisible(m_isExpanded);
            }
            if (m_onToggleCallback) {
                m_onToggleCallback(m_isExpanded);
            }
        }
    }

    void UIDropdown::update(float deltaTime) {
        UIElement::update(deltaTime);
        if (m_contentContainer) {
            if (m_contentContainer->isParentExpired()) {
                m_contentContainer->setParent(shared_from_this());
            }
            if (m_isExpanded) {
                m_contentContainer->update(deltaTime);
            }
        }
    }

    void UIDropdown::onEvent(const SDL_Event& event) {
        UIElement::onEvent(event);
        if (m_contentContainer) {
            if (m_contentContainer->isParentExpired()) {
                m_contentContainer->setParent(shared_from_this());
            }
        }

        if (m_isExpanded && m_contentContainer) {
            m_contentContainer->onEvent(event);
        }

        // header click
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                glm::vec2 absPos = getAbsolutePosition();
                float mouseX = event.button.x;
                float mouseY = event.button.y;

                if (mouseX >= absPos.x && mouseX <= absPos.x + m_size.x && mouseY >= absPos.y && mouseY <= absPos.y + m_headerHeight) {
                    setExpanded(!m_isExpanded);
                }
            }
        }
    }

    void UIDropdown::generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) {
        if (!m_visible) return;

        if (m_contentContainer) {
            if (m_contentContainer->isParentExpired()) {
                m_contentContainer->setParent(shared_from_this());
            }
        }

        glm::vec2 absPos = getAbsolutePosition();
        uint16_t headerIdx = static_cast<uint16_t>(vertices.size());
        glm::vec4 headerColor(0.016f, 0.018f, 0.023f, 1.0f);

        vertices.push_back(UIVertex{.pos = absPos, .color = headerColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + m_size.x, absPos.y), .color = headerColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + m_size.x, absPos.y + m_headerHeight), .color = headerColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x, absPos.y + m_headerHeight), .color = headerColor, .uv = glm::vec2(-1.0f)});
        indices.push_back(headerIdx + 0); indices.push_back(headerIdx + 1); indices.push_back(headerIdx + 2);
        indices.push_back(headerIdx + 0); indices.push_back(headerIdx + 2); indices.push_back(headerIdx + 3);

        if (m_fontLoader) {
            std::string indicator = m_isExpanded ? "v  " : ">  ";
            glm::vec4 textColor(0.85f, 0.85f, 0.90f, 1.0f);
            m_fontLoader->generateTextGeometry(indicator + m_title, glm::vec2(absPos.x + 10.0f, absPos.y + 18.0f), textColor, vertices, indices);
        }

        if (m_isExpanded && m_contentContainer) {
            m_contentContainer->generateGeometry(vertices, indices);
        }
    }

    void UIDropdown::addContentElement(const std::shared_ptr<UIElement>& element) {
        if (m_contentContainer) {
            if (m_contentContainer->isParentExpired()) {
                m_contentContainer->setParent(shared_from_this());
            }
            m_contentContainer->addChild(element);
        }
    }

}