#pragma once

#include "ui/ui_element.hpp"
#include "resources/font_loader.hpp"
#include <SDL3/SDL.h>
#include <functional>
#include <memory>
#include <string>

namespace slate {
    
    class UIInputBox : public UIElement {
    public:
        UIInputBox(const std::string& name, glm::vec2 position, glm::vec2 size, float initialValue = 0.0f);
        UIInputBox(const std::string& name, glm::vec2 position, glm::vec2 size, const std::string& initialText);

        void setNumberMode(bool enabled, float minVal = -10000.0f, float maxVal = 10000.0f, float dragSpeed = 0.05f);
        void setDecimals(int decimals);

        void setValue(float value);
        void setValueWithoutCallback(float value);
        float getValue() const { return m_value; }

        void setText(const std::string& text);
        void setTextWithoutCallback(const std::string& text);
        const std::string& getText() const { return m_text; }

        void setMaxLength(size_t n) { m_maxLength = n; }
        void setCharFilter(std::function<bool(char)> filter) { m_charFilter = std::move(filter); }
        bool isFocused() const { return m_isFocused; }

        void setFontLoader(std::shared_ptr<FontLoader> fontLoader) { m_fontLoader = fontLoader; }
        void setOnValueChanged(std::function<void(float)> cb) { m_onValueChanged = std::move(cb); }
        void setOnTextChanged(std::function<void(const std::string&)> cb) { m_onTextChanged = std::move(cb); }

        void setBackgroundColor(glm::vec4 c) { m_bgColor = c; }
        void setFocusedColor(glm::vec4 c) { m_focusedBgColor = c; }
        void setTextColor(glm::vec4 c) { m_textColor = c; }

        void update(float deltaTime) override;
        void onEvent(const SDL_Event& event) override;
        void generateGeometry(std::vector<UIVertex>& vertices, std::vector<UIIndex>& indices) override;

    private:
        bool contains(float x, float y) const;

        void beginEdit(SDL_Window* window);
        void endEdit(bool commit);
        void typeText(const char* utf8);
        void handleKey(const SDL_KeyboardEvent& key);
        void resetCaretBlink();

        void updateTextFromValue();
        std::string visibleText(float maxWidth) const;

        // value
        bool  m_isNumberMode = true;
        float m_value = 0.0f;
        float m_minValue = -10000.0f;
        float m_maxValue = 10000.0f;
        float m_dragSpeed = 0.05f;
        int   m_decimals = 3;

        std::string m_text = "0";
        size_t m_maxLength = 0;
        std::function<bool(char)> m_charFilter;

        // interaction
        bool m_isFocused = false;
        bool m_isHovered = false;
        bool m_isPressed = false;
        bool m_isScrubbing = false;
        float m_dragTravel = 0.0f;
        bool m_selectAll = false;

        SDL_Window* m_window = nullptr;
        float m_valueBeforeEdit = 0.0f;
        std::string m_textBeforeEdit;

        // caret
        float m_caretTimer = 0.0f;
        bool  m_caretVisible = true;

        // look
        glm::vec4 m_bgColor = glm::vec4(0.032f, 0.036f, 0.046f, 1.0f);
        glm::vec4 m_focusedBgColor = glm::vec4(0.06f, 0.07f, 0.09f, 1.0f);
        glm::vec4 m_textColor = glm::vec4(0.85f, 0.85f, 0.90f, 1.0f);

        std::shared_ptr<FontLoader> m_fontLoader;
        std::function<void(float)> m_onValueChanged;
        std::function<void(const std::string&)> m_onTextChanged;
    };

}
