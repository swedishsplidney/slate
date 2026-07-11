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

}