#include "ui_color_picker.hpp"
#include <SDL3/SDL.h>
#include <algorithm>

namespace slate {

    UIColorPicker::UIColorPicker(const std::string& name, glm::vec2 position, glm::vec2 size)
        : UIElement(name, position, size) {
        setDrawsBackground(true);
    }

    void UIColorPicker::setColorValue(const glm::vec4& color) {
        m_color = color;
        if (m_rgbInputBoxes[0]) m_rgbInputBoxes[0]->setValueWithoutCallback(color.r);
        if (m_rgbInputBoxes[1]) m_rgbInputBoxes[1]->setValueWithoutCallback(color.g);
        if (m_rgbInputBoxes[2]) m_rgbInputBoxes[2]->setValueWithoutCallback(color.b);
    }

    void UIColorPicker::onEvent(const SDL_Event& event) {
        UIElement::onEvent(event);

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
            glm::vec2 mousePos(event.button.x, event.button.y);
            glm::vec2 absPos = getAbsolutePosition();

            bool clickedSwatch = (mousePos.x >= absPos.x && mousePos.x <= absPos.x + m_size.x &&
                                  mousePos.y >= absPos.y && mousePos.y <= absPos.y + m_size.y);

            if (clickedSwatch) {
                m_isOpen = !m_isOpen;
                if (m_isOpen && m_children.empty()) {
                    float popupWidth = m_size.x;
                    float fieldWidth = (popupWidth - 24.0f) / 3.0f;

                    for (int i = 0; i < 3; ++i) {
                        float fx = 8.0f + (i * (fieldWidth + 4.0f));
                        auto inputBox = std::make_shared<UIInputBox>(
                            "ColorInput_" + std::to_string(i),
                            glm::vec2(fx, 36.0f),
                            glm::vec2(fieldWidth, 22.0f),
                            (i == 0) ? m_color.r : (i == 1) ? m_color.g : m_color.b
                        );
                        inputBox->setFontLoader(m_fontLoader);
                        inputBox->setDrawsBackground(true);
                        inputBox->setColor(glm::vec4(0.08f, 0.09f, 0.12f, 1.0f));

                        std::weak_ptr<UIInputBox> weakBox = inputBox;
                        inputBox->setOnValueChanged([this, i, weakBox](float val) {
                            float clamped = std::clamp(val, 0.0f, 1.0f);
                            if (clamped != val) {
                                if (auto box = weakBox.lock()) {
                                    box->setValueWithoutCallback(clamped);
                                }
                            }

                            if (i == 0) m_color.r = clamped;
                            else if (i == 1) m_color.g = clamped;
                            else if (i == 2) m_color.b = clamped;
                            m_color.a = 1.0f;

                            if (m_onColorChanged) {
                                m_onColorChanged(m_color);
                            }
                        });

                        addChild(inputBox);
                        m_rgbInputBoxes[i] = inputBox;
                    }
                }
            }
        }
    }

    void UIColorPicker::generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) {
        if (!m_visible) return;

        glm::vec2 absPos = getAbsolutePosition();
        uint16_t idx = static_cast<uint16_t>(vertices.size());

        vertices.push_back(UIVertex{.pos = absPos, .color = m_color, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + m_size.x, absPos.y), .color = m_color, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + m_size.x, absPos.y + m_size.y), .color = m_color, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x, absPos.y + m_size.y), .color = m_color, .uv = glm::vec2(-1.0f)});

        indices.push_back(idx + 0); indices.push_back(idx + 1); indices.push_back(idx + 2);
        indices.push_back(idx + 0); indices.push_back(idx + 2); indices.push_back(idx + 3);

        if (m_isOpen) {
            float popupHeight = 70.0f;
            glm::vec2 popupPos(absPos.x, absPos.y + m_size.y + 4.0f);
            glm::vec4 popupBgColor(0.06f, 0.07f, 0.10f, 1.0f);

            uint16_t popIdx = static_cast<uint16_t>(vertices.size());
            vertices.push_back(UIVertex{.pos = popupPos, .color = popupBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(popupPos.x + m_size.x, popupPos.y), .color = popupBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(popupPos.x + m_size.x, popupPos.y + popupHeight), .color = popupBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(popupPos.x, popupPos.y + popupHeight), .color = popupBgColor, .uv = glm::vec2(-1.0f)});

            indices.push_back(popIdx + 0); indices.push_back(popIdx + 1); indices.push_back(popIdx + 2);
            indices.push_back(popIdx + 0); indices.push_back(popIdx + 2); indices.push_back(popIdx + 3);

            if (m_fontLoader) {
                m_fontLoader->generateTextGeometry("RGB Color Picker", glm::vec2(popupPos.x + 8.0f, popupPos.y + 18.0f), glm::vec4(0.7f, 0.7f, 0.75f, 1.0f), vertices, indices);
            }

            for (auto& child : m_children) {
                if (child) {
                    child->generateGeometry(vertices, indices);
                }
            }
        }
    }
}