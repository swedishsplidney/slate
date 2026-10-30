#include "ui_input_box.hpp"
#include "ui/ui_draw.hpp"
#include "ui/ui_theme.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace slate {

    namespace {
        constexpr float kPadX = 6.0f;
        constexpr float kDragThreshold = 3.0f;
        constexpr float kBlinkPeriod = 1.0f;

        bool parseFloat(const std::string& s, float& out) {
            if (s.empty()) return false;
            char* end = nullptr;
            const float v = std::strtof(s.c_str(), &end);
            if (end == s.c_str() || *end != '\0' || !std::isfinite(v)) return false;
            out = v;
            return true;
        }
    }

    // construction

    UIInputBox::UIInputBox(const std::string& name, glm::vec2 position, glm::vec2 size, float initialValue)
        : UIElement(name, position, size), m_isNumberMode(true), m_value(initialValue) {
        updateTextFromValue();
    }

    UIInputBox::UIInputBox(const std::string& name, glm::vec2 position, glm::vec2 size, const std::string& initialText)
        : UIElement(name, position, size), m_isNumberMode(false), m_text(initialText) {
    }

    void UIInputBox::setNumberMode(bool enabled, float minVal, float maxVal, float dragSpeed) {
        m_isNumberMode = enabled;
        m_minValue = minVal;
        m_maxValue = maxVal;
        m_dragSpeed = dragSpeed;
        if (enabled) {
            float parsed;
            if (parseFloat(m_text, parsed)) m_value = parsed;
            m_value = std::clamp(m_value, m_minValue, m_maxValue);
            updateTextFromValue();
        }
        markDirty();
    }

    void UIInputBox::setDecimals(int decimals) {
        m_decimals = std::clamp(decimals, 0, 6);
        if (m_isNumberMode && !m_isFocused) updateTextFromValue();
        markDirty();
    }

    // value

    void UIInputBox::setValue(float value) {
        value = std::clamp(value, m_minValue, m_maxValue);
        if (value == m_value) return;
        m_value = value;
        if (!m_isFocused) updateTextFromValue();
        markDirty();
        if (m_onValueChanged) m_onValueChanged(m_value);
    }

    void UIInputBox::setValueWithoutCallback(float value) {
        m_value = std::clamp(value, m_minValue, m_maxValue);
        if (m_isFocused) return;
        updateTextFromValue();
        markDirty();
    }

    void UIInputBox::setText(const std::string& text) {
        m_text = text;
        if (m_isNumberMode) {
            float parsed;
            if (parseFloat(m_text, parsed)) m_value = std::clamp(parsed, m_minValue, m_maxValue);
        }
        markDirty();
        if (m_onTextChanged) m_onTextChanged(m_text);
    }

    void UIInputBox::setTextWithoutCallback(const std::string& text) {
        if (m_isFocused) return;
        m_text = text;
        markDirty();
    }

    void UIInputBox::updateTextFromValue() {
        char buf[48];

        float displayValue = m_value;
        const float precision = std::pow(10.0f, m_decimals);

        if (std::abs(displayValue) < 0.5f / precision) {
            displayValue = 0.0f;
        }

        std::snprintf(
            buf,
            sizeof(buf),
            "%.*f",
            m_decimals,
            static_cast<double>(displayValue)
        );

        m_text = buf;
    }

    void UIInputBox::beginEdit(SDL_Window* window) {
        m_window = window;
        m_isFocused = true;
        m_selectAll = true;
        m_valueBeforeEdit = m_value;
        m_textBeforeEdit = m_text;
        SDL_StartTextInput(window);
        resetCaretBlink();
    }

    void UIInputBox::endEdit(bool commit) {
        if (!m_isFocused) return;
        m_isFocused = false;
        m_selectAll = false;
        if (m_window) SDL_StopTextInput(m_window);

        if (m_isNumberMode) {
            float parsed = m_valueBeforeEdit;
            if (!commit || !parseFloat(m_text, parsed)) parsed = m_valueBeforeEdit;
            m_value = std::clamp(parsed, m_minValue, m_maxValue);
            updateTextFromValue();
            if (m_value != m_valueBeforeEdit && m_onValueChanged) m_onValueChanged(m_value);
        } else {
            if (!commit) m_text = m_textBeforeEdit;
            if (m_text != m_textBeforeEdit && m_onTextChanged) m_onTextChanged(m_text);
        }
        markDirty();
    }

    // events

    bool UIInputBox::contains(float x, float y) const {
        const glm::vec2 p = getAbsolutePosition();
        return x >= p.x && x <= p.x + m_size.x && y >= p.y && y <= p.y + m_size.y;
    }

    void UIInputBox::onEvent(const SDL_Event& event) {
        UIElement::onEvent(event);

        switch (event.type) {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button != SDL_BUTTON_LEFT) break;
                if (contains(event.button.x, event.button.y)) {
                    if (!m_isFocused) {
                        m_isPressed = true;
                        m_isScrubbing = false;
                        m_dragTravel = 0.0f;
                    }
                } else {
                    endEdit(true);
                }
                break;

            case SDL_EVENT_MOUSE_MOTION: {
                const bool inside = contains(event.motion.x, event.motion.y);
                if (inside != m_isHovered) { m_isHovered = inside; markDirty(); }

                if (m_isPressed && m_isNumberMode) {
                    m_dragTravel += std::abs(event.motion.xrel);
                    if (!m_isScrubbing && m_dragTravel > kDragThreshold) m_isScrubbing = true;

                    if (m_isScrubbing) {
                        const SDL_Keymod mod = SDL_GetModState();
                        float speed = m_dragSpeed;
                        if (mod & SDL_KMOD_SHIFT)     speed *= 0.1f;
                        else if (mod & SDL_KMOD_CTRL) speed *= 10.0f;
                        setValue(m_value + event.motion.xrel * speed);
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button != SDL_BUTTON_LEFT || !m_isPressed) break;
                m_isPressed = false;
                if (!m_isScrubbing && contains(event.button.x, event.button.y)) {
                    if (SDL_Window* w = SDL_GetWindowFromID(event.button.windowID)) beginEdit(w);
                }
                m_isScrubbing = false;
                break;

            case SDL_EVENT_TEXT_INPUT:
                if (m_isFocused && event.text.text) typeText(event.text.text);
                break;

            case SDL_EVENT_KEY_DOWN:
                if (m_isFocused) handleKey(event.key);
                break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                endEdit(true);
                break;

            default:
                break;
        }
    }

    void UIInputBox::typeText(const char* utf8) {
        const std::string base = m_selectAll ? std::string() : m_text;
        std::string next = base;

        for (const char* p = utf8; *p; ++p) {
            const unsigned char u = static_cast<unsigned char>(*p);
            if (u < 32 || u >= 127) continue;
            if (m_maxLength && next.size() >= m_maxLength) break;

            bool ok = true;
            if (m_charFilter) {
                ok = m_charFilter(*p);
            } else if (m_isNumberMode) {
                if (*p >= '0' && *p <= '9') ok = true;
                else if (*p == '.')         ok = next.find('.') == std::string::npos;
                else if (*p == '-')         ok = next.empty();
                else                        ok = false;
            }
            if (ok) next += *p;
        }

        if (next == base) return;
        m_text = next;
        m_selectAll = false;
        resetCaretBlink();
    }

    void UIInputBox::handleKey(const SDL_KeyboardEvent& key) {
        const bool ctrl = (key.mod & SDL_KMOD_CTRL) != 0;

        switch (key.key) {
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                endEdit(true);
                return;
            case SDLK_ESCAPE:
                endEdit(false);
                return;
            case SDLK_BACKSPACE:
                if (m_selectAll)          { m_text.clear(); m_selectAll = false; }
                else if (!m_text.empty()) { m_text.pop_back(); }
                break;
            case SDLK_A:
                if (ctrl) m_selectAll = true;
                break;
            case SDLK_C:
                if (ctrl) SDL_SetClipboardText(m_text.c_str());
                return;
            case SDLK_V:
                if (ctrl) {
                    if (char* clip = SDL_GetClipboardText()) { typeText(clip); SDL_free(clip); }
                }
                return;
            default:
                return;
        }
        resetCaretBlink();
    }

    // caret

    void UIInputBox::resetCaretBlink() {
        m_caretTimer = 0.0f;
        m_caretVisible = true;
        markDirty();
    }

    void UIInputBox::update(float deltaTime) {
        UIElement::update(deltaTime);
        if (!m_isFocused) return;

        m_caretTimer += deltaTime;
        const bool visible = std::fmod(m_caretTimer, kBlinkPeriod) < kBlinkPeriod * 0.5f;
        if (visible != m_caretVisible) {
            m_caretVisible = visible;
            markDirty();
        }
    }

    // drawing

    std::string UIInputBox::visibleText(float maxWidth) const {
        if (!m_fontLoader || m_fontLoader->measureText(m_text) <= maxWidth) return m_text;

        std::string s = m_text;
        while (!s.empty() && m_fontLoader->measureText(s) > maxWidth) {
            if (m_isFocused) s.erase(0, 1);
            else             s.pop_back();
        }
        return s;
    }

    void UIInputBox::generateGeometry(std::vector<UIVertex>& vertices, std::vector<UIIndex>& indices) {
        if (!m_visible) return;

        const glm::vec2 p = getAbsolutePosition();

        glm::vec4 bg = m_bgColor;
        if (m_isFocused)      bg = m_focusedBgColor;
        else if (m_isHovered) bg = m_bgColor + glm::vec4(0.014f, 0.015f, 0.019f, 0.0f);

        pushQuad(vertices, indices, p, m_size, bg, m_cornerRadius);

        if (!m_fontLoader) return;

        const std::string shown = visibleText(m_size.x - 2.0f * kPadX);
        const float textW = m_fontLoader->measureText(shown);
        const float textX = p.x + kPadX;
        const float baseline = p.y + m_size.y * 0.5f + 5.0f;

        if (m_isFocused && m_selectAll && !shown.empty()) {
            pushQuad(vertices, indices,
                     glm::vec2(textX - 2.0f, p.y + 3.0f), glm::vec2(textW + 4.0f, m_size.y - 6.0f),
                     theme::selection, 2.0f);
        }

        m_fontLoader->generateTextGeometry(shown, glm::vec2(textX, baseline), m_textColor, vertices, indices);

        if (m_isFocused && !m_selectAll && m_caretVisible) {
            pushQuad(vertices, indices,
                     glm::vec2(textX + textW + 1.0f, p.y + 4.0f), glm::vec2(1.0f, m_size.y - 8.0f),
                     m_textColor);
        }
    }

}
