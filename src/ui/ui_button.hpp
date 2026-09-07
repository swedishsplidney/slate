#pragma once

#include "ui/ui_element.hpp"
#include "resources/font_loader.hpp"
#include <functional>
#include <string>

namespace slate {

    class UIButton : public UIElement {
    public:
        UIButton(const std::string& name, glm::vec2 position, glm::vec2 size, const std::string& text, std::function<void()> onClick)
            : UIElement(name, position, size), m_text(text), m_onClickCallback(onClick) {}

        void setFontLoader(std::shared_ptr<FontLoader> fontLoader) { m_fontLoader = fontLoader; }
        void setText(const std::string& text) { m_text = text; }

        void generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) override {
            UIElement::generateGeometry(vertices, indices);

            if (!m_text.empty() && m_fontLoader) {
                glm::vec2 absPos = getAbsolutePosition();
                glm::vec2 textPos = glm::vec2(absPos.x + 10.0f, absPos.y + 15.0f);
                glm::vec4 textColor(0.9f, 0.9f, 0.95f, 1.0f);
                m_fontLoader->generateTextGeometry(m_text, textPos, textColor, vertices, indices);
            }
        }

        void onEvent(const SDL_Event& event) override {
            UIElement::onEvent(event);

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    glm::vec2 absPos = getAbsolutePosition();
                    float mouseX = event.button.x;
                    float mouseY = event.button.y;

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
        std::string m_text;
        std::shared_ptr<FontLoader> m_fontLoader;
        std::function<void()> m_onClickCallback;
    };

}