#include "ui/ui_element.hpp"
#include <SDL3/SDL_events.h>

namespace slate {
    
    UIElement::UIElement(const std::string& name, glm::vec2 position, glm::vec2 size)
        : m_name(name), m_position(position), m_size(size) {}

    void UIElement::addChild(std::shared_ptr<UIElement> child) {
        child->m_parent = shared_from_this();
        m_children.push_back(child);
    }

    glm::vec2 UIElement::getAbsolutePosition() const {
        if (auto parentPtr = m_parent.lock()) {
            return parentPtr->getAbsolutePosition() + m_position;
        }
        return m_position;
    }

    void UIElement::update(float deltaTime) {
        for (auto& child : m_children) {
            child->update(deltaTime);
        }
    }

    void UIElement::onEvent(const SDL_Event& event) {
        for (auto& child : m_children) {
            child->onEvent(event);
        }
    }

    void UIElement::generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) {
        if (!m_visible) return;

        if (m_drawsBackground) {
            glm::vec2 absPos = getAbsolutePosition();
            uint16_t baseIndex = static_cast<uint16_t>(vertices.size());

            vertices.push_back(UIVertex{.pos = absPos, .color = m_color, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = absPos + glm::vec2(m_size.x, 0.0f), .color = m_color, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = absPos + m_size, .color = m_color, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = absPos + glm::vec2(0.0f, m_size.y), .color = m_color, .uv = glm::vec2(-1.0f)});

            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 3);
        }

        for (auto& child : m_children) {
            if (child) {
                child->generateGeometry(vertices, indices);
            }
        }
    }

}