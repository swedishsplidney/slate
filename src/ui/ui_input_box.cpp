#include "ui_input_box.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace slate {

    UIInputBox::UIInputBox(const std::string& name, glm::vec2 position, glm::vec2 size, float initialValue)
        : UIElement(name, position, size), m_value(initialValue), m_isNumberMode(true) {
        updateTextFromValue();
    }

    UIInputBox::UIInputBox(const std::string& name, glm::vec2 position, glm::vec2 size, const std::string& initialText)
        : UIElement(name, position, size), m_text(initialText), m_isNumberMode(false) {
    }

    void UIInputBox::setNumberMode(bool enabled, float minVal, float maxVal, float step) {
        m_isNumberMode = enabled;
        m_minValue = minVal;
        m_maxValue = maxVal;
        m_scrubSensitivity = step;
        if (m_isNumberMode) {
            updateValueFromText();
            updateTextFromValue();
        }
    }

    void UIInputBox::setValue(float value) {
        m_value = std::clamp(value, m_minValue, m_maxValue);
        updateTextFromValue();
        if (m_onValueChanged) {
            m_onValueChanged(m_value);
        }
    }

    void UIInputBox::setText(const std::string& text) {
        m_text = text;
        if (m_isNumberMode) {
            updateValueFromText();
        }
        if (m_onTextChanged) {
            m_onTextChanged(m_text);
        }
    }

    void UIInputBox::updateTextFromValue() {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(3) << m_value;
        m_text = stream.str();
    }

    void UIInputBox::updateValueFromText() {
        try {
            if (!m_text.empty() && m_text != "-") {
                float parsed = std::stof(m_text);
                m_value = std::clamp(parsed, m_minValue, m_maxValue);
            }
        } catch (...) {
        }
    }

    void UIInputBox::onEvent(const SDL_Event& event) {
        UIElement::onEvent(event);
        glm::vec2 absPos = getAbsolutePosition();

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                float mouseX = event.button.x;
                float mouseY = event.button.y;

                bool inside = (mouseX >= absPos.x && mouseX <= absPos.x + m_size.x &&
                               mouseY >= absPos.y && mouseY <= absPos.y + m_size.y);

                if (inside) {
                    m_isDragging = true;
                    m_dragStartX = mouseX;
                    m_dragStartValue = m_value;
                    m_totalDragDelta = 0.0f;
                } else {
                    if (m_isFocused) {
                        m_isFocused = false;
                        if (m_isNumberMode) {
                            updateValueFromText();
                            updateTextFromValue();
                        }
                    }
                }
            }
        } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            if (m_isDragging && m_isNumberMode) {
                float mouseX = event.motion.x;
                float deltaX = mouseX - m_dragStartX;
                m_totalDragDelta += std::abs(deltaX);

                if (m_totalDragDelta > 2.0f) {
                    float newValue = m_dragStartValue + (deltaX * m_scrubSensitivity);
                    setValue(newValue);
                }
            }
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            if (event.button.button == SDL_BUTTON_LEFT && m_isDragging) {
                m_isDragging = false;
                float mouseX = event.button.x;
                float mouseY = event.button.y;
                bool inside = (mouseX >= absPos.x && mouseX <= absPos.x + m_size.x &&
                               mouseY >= absPos.y && mouseY <= absPos.y + m_size.y);

                if (inside && m_totalDragDelta <= 2.0f) {
                    m_isFocused = true;
                }
            }
        } else if (m_isFocused) {
            if (event.type == SDL_EVENT_TEXT_INPUT) {
                const char* inputStr = event.text.text;
                if (inputStr) {
                    for (int i = 0; inputStr[i] != '\0'; ++i) {
                        char c = inputStr[i];
                        if (m_isNumberMode) {
                            if ((c >= '0' && c <= '9') || c == '.' || (c == '-' && m_text.empty())) {
                                m_text += c;
                            }
                        } else {
                            m_text += c;
                        }
                    }
                    if (m_isNumberMode) updateValueFromText();
                    if (m_onTextChanged) m_onTextChanged(m_text);
                    if (m_isNumberMode && m_onValueChanged) m_onValueChanged(m_value);
                }
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_BACKSPACE && !m_text.empty()) {
                    m_text.pop_back();
                    if (m_isNumberMode) updateValueFromText();
                    if (m_onTextChanged) m_onTextChanged(m_text);
                    if (m_isNumberMode && m_onValueChanged) m_onValueChanged(m_value);
                } else if (event.key.key == SDLK_RETURN || event.key.key == SDLK_KP_ENTER || event.key.key == SDLK_ESCAPE) {
                    m_isFocused = false;
                    if (m_isNumberMode) {
                        updateValueFromText();
                        updateTextFromValue();
                        if (m_onValueChanged) m_onValueChanged(m_value);
                    }
                }
            }
        }
    }

    void UIInputBox::generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) {
        if (!m_visible) return;

        glm::vec2 absPos = getAbsolutePosition();
        glm::vec4 activeColor = m_isFocused ? m_focusedBgColor : m_bgColor;

        uint16_t bIdx = static_cast<uint16_t>(vertices.size());
        vertices.push_back(UIVertex{.pos = absPos, .color = activeColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + m_size.x, absPos.y), .color = activeColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + m_size.x, absPos.y + m_size.y), .color = activeColor, .uv = glm::vec2(-1.0f)});
        vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x, absPos.y + m_size.y), .color = activeColor, .uv = glm::vec2(-1.0f)});

        indices.push_back(bIdx + 0); indices.push_back(bIdx + 1); indices.push_back(bIdx + 2);
        indices.push_back(bIdx + 0); indices.push_back(bIdx + 2); indices.push_back(bIdx + 3);

        if (m_fontLoader) {
            std::string displayStr = m_text;
            if (displayStr.length() > 6) displayStr = displayStr.substr(0, 6);
            m_fontLoader->generateTextGeometry(displayStr, glm::vec2(absPos.x + 6.0f, absPos.y + 16.0f), m_textColor, vertices, indices);
        }
    }

}