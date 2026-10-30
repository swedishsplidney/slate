#include "ui/ui_element.hpp"
#include "ui/ui_draw.hpp"
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

    void UIElement::markDirty() {
        if (auto p = m_parent.lock()) p->markDirty();
        else if (m_onDirty) m_onDirty();
    }
    void UIElement::update(float dt)            { for (auto& c : m_children) if (c && c->m_visible) c->update(dt); }

    void UIElement::onEvent(const SDL_Event& e) { for (auto& c : m_children) if (c && c->m_visible) c->onEvent(e); }

    void UIElement::generateGeometry(std::vector<UIVertex>& v, std::vector<UIIndex>& i) {
        if (!m_visible || !m_drawsBackground) return;
        pushQuad(v, i, getAbsolutePosition(), m_size, m_color, m_cornerRadius);
    }

}