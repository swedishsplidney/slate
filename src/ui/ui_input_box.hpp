#pragma once

#include "ui/ui_element.hpp"
#include "resources/font_loader.hpp"
#include <functional>
#include <string>
#include <memory>

namespace slate {

    class UIInputBox : public UIElement {
    public:
        UIInputBox(const std::string& name, glm::vec2 position, glm::vec2 size, float initialValue = 0.0f);
        UIInputBox(const std::string& name, glm::vec2 position, glm::vec2 size, const std::string& initialText);

        void setNumberMode(bool enabled, float minVal = -10000.0f, float maxVal = 10000.0f, float step = 0.05f);
        void setValue(float value);
        float getValue() const { return m_value; }

        void setText(const std::string& text);
        std::string getText() const { return m_text; }

        void setFontLoader(std::shared_ptr<FontLoader> fontLoader) { m_fontLoader = fontLoader; }

        void setOnValueChanged(std::function<void(float)> callback) { m_onValueChanged = callback; }
        void setOnTextChanged(std::function<void(const std::string&)> callback) { m_onTextChanged = callback; }

        void setBackgroundColor(glm::vec4 color) { m_bgColor = color; }
        void setFocusedColor(glm::vec4 color) { m_focusedBgColor = color; }
        void setTextColor(glm::vec4 color) { m_textColor = color; }

        void onEvent(const SDL_Event& event) override;
        void generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) override;

    private:
        bool m_isNumberMode = true;
        float m_value = 0.0f;
        float m_minValue = -10000.0f;
        float m_maxValue = 10000.0f;
        float m_scrubSensitivity = 0.05f;

        std::string m_text = "0.000";
        bool m_isFocused = false;
        bool m_isDragging = false;
        float m_dragStartX = 0.0f;
        float m_dragStartValue = 0.0f;
        float m_totalDragDelta = 0.0f;

        glm::vec4 m_bgColor = glm::vec4(0.032f, 0.036f, 0.046f, 1.0f);
        glm::vec4 m_focusedBgColor = glm::vec4(0.06f, 0.07f, 0.09f, 1.0f);
        glm::vec4 m_textColor = glm::vec4(0.85f, 0.85f, 0.90f, 1.0f);

        std::shared_ptr<FontLoader> m_fontLoader;
        std::function<void(float)> m_onValueChanged;
        std::function<void(const std::string&)> m_onTextChanged;

        void updateTextFromValue();
        void updateValueFromText();

        bool m_hasBeenEdited = false;
    };

}