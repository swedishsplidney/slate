#pragma once

#include "ui/ui_element.hpp"
#include <functional>

namespace slate {

    class UIButton : public UIElement {
    public:
        UIButton(const std::string& name, glm::vec2 position, glm::vec2 size, std::function<void()> onClick)
            : UIElement(name, position, size), m_onClickCallback(onClick) {}

        void onEvent(const SDL_Event& event) override {
            UIElement::onEvent(event);

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    glm::vec2 absPos = getAbsolutePosition();
                    float mouseX = event.button.x;
                    float mouseY = event.button.y;

                    // aabb collision check to see if click is inside bounds
                    if (mouseX >= absPos.x && mouseX <= absPos.x + m_size.x &&
                        mouseY >= absPos.y && mouseY <= absPos.y + m_size.y) {

                        if (m_onClickCallback) {
                            m_onClickCallback();
                        }
                    }
                }
            }
        }
    private:
        std::function<void()> m_onClickCallback;
    };

}