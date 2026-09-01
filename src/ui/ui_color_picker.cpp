#include "ui_color_picker.hpp"
#include <SDL3/SDL.h>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace slate {

    UIColorPicker::UIColorPicker(const std::string& name, glm::vec2 position, glm::vec2 size)
        : UIElement(name, position, size) {
        setDrawsBackground(true);
    }

    float UIColorPicker::getExpandedHeight() const {
        if (!m_isOpen) return m_size.y;
        return m_size.y + 195.0f;
    }

    void UIColorPicker::setColorValue(const glm::vec4& color) {
        m_color = color;
        updateInputBoxes();
    }

    void UIColorPicker::updateInputBoxes() {
        if (m_rgbInputBoxes[0]) m_rgbInputBoxes[0]->setValueWithoutCallback(m_color.r);
        if (m_rgbInputBoxes[1]) m_rgbInputBoxes[1]->setValueWithoutCallback(m_color.g);
        if (m_rgbInputBoxes[2]) m_rgbInputBoxes[2]->setValueWithoutCallback(m_color.b);

        float r = m_color.r, g = m_color.g, b = m_color.b;
        float k = 1.0f - std::max({r, g, b});
        float c = (1.0f - r - k) / (1.0f - k + 0.00001f);
        float m = (1.0f - g - k) / (1.0f - k + 0.00001f);
        float y = (1.0f - b - k) / (1.0f - k + 0.00001f);
        m_cmykValues = glm::vec4(std::clamp(c, 0.0f, 1.0f), std::clamp(m, 0.0f, 1.0f), std::clamp(y, 0.0f, 1.0f), std::clamp(k, 0.0f, 1.0f));

        for (int i = 0; i < 4; ++i) {
            if (m_cmykInputBoxes[i]) {
                m_cmykInputBoxes[i]->setValueWithoutCallback(m_cmykValues[i]);
            }
        }

        int hexVal = ((int)(std::clamp(m_color.r, 0.0f, 1.0f) * 255) << 16) |
                     ((int)(std::clamp(m_color.g, 0.0f, 1.0f) * 255) << 8) |
                      (int)(std::clamp(m_color.b, 0.0f, 1.0f) * 255);
        if (m_hexInputBox) {
            m_hexInputBox->setValueWithoutCallback((float)hexVal);
        }
    }

    void UIColorPicker::rebuildPopupElements() {
        m_children.clear();
        float popupWidth = m_size.x;

        // rgb
        float fieldWidthRGB = (popupWidth - 24.0f) / 3.0f;
        for (int i = 0; i < 3; ++i) {
            float fx = 8.0f + (i * (fieldWidthRGB + 4.0f));
            float initialVal = (i == 0) ? m_color.r : (i == 1) ? m_color.g : m_color.b;
            auto inputBox = std::make_shared<UIInputBox>(
                "RGBInput_" + std::to_string(i),
                glm::vec2(fx, 40.0f),
                glm::vec2(fieldWidthRGB, 22.0f),
                initialVal
            );
            inputBox->setFontLoader(m_fontLoader);
            inputBox->setDrawsBackground(true);
            inputBox->setColor(glm::vec4(0.08f, 0.09f, 0.12f, 1.0f));

            std::weak_ptr<UIInputBox> weakBox = inputBox;
            inputBox->setOnValueChanged([this, i, weakBox](float val) {
                float clamped = std::clamp(val, 0.0f, 1.0f);
                if (clamped != val) {
                    if (auto box = weakBox.lock()) box->setValueWithoutCallback(clamped);
                }
                if (i == 0) m_color.r = clamped;
                else if (i == 1) m_color.g = clamped;
                else if (i == 2) m_color.b = clamped;

                updateInputBoxes();
                if (m_onColorChanged) m_onColorChanged(m_color);
            });

            addChild(inputBox);
            m_rgbInputBoxes[i] = inputBox;
        }

        // cmyk
        float fieldWidthCMYK = (popupWidth - 28.0f) / 4.0f;
        float r = m_color.r, g = m_color.g, b = m_color.b;
        float k = 1.0f - std::max({r, g, b});
        float c = (1.0f - r - k) / (1.0f - k + 0.00001f);
        float m = (1.0f - g - k) / (1.0f - k + 0.00001f);
        float y = (1.0f - b - k) / (1.0f - k + 0.00001f);
        m_cmykValues = glm::vec4(std::clamp(c,0.f,1.f), std::clamp(m,0.f,1.f), std::clamp(y,0.f,1.f), std::clamp(k,0.f,1.f));

        for (int i = 0; i < 4; ++i) {
            float fx = 8.0f + (i * (fieldWidthCMYK + 4.0f));
            auto inputBox = std::make_shared<UIInputBox>(
                "CMYKInput_" + std::to_string(i),
                glm::vec2(fx, 95.0f),
                glm::vec2(fieldWidthCMYK, 22.0f),
                m_cmykValues[i]
            );
            inputBox->setFontLoader(m_fontLoader);
            inputBox->setDrawsBackground(true);
            inputBox->setColor(glm::vec4(0.08f, 0.09f, 0.12f, 1.0f));

            std::weak_ptr<UIInputBox> weakBox = inputBox;
            inputBox->setOnValueChanged([this, i, weakBox](float val) {
                float clamped = std::clamp(val, 0.0f, 1.0f);
                if (clamped != val) {
                    if (auto box = weakBox.lock()) box->setValueWithoutCallback(clamped);
                }
                m_cmykValues[i] = clamped;

                float cc = m_cmykValues.x;
                float mm = m_cmykValues.y;
                float yy = m_cmykValues.z;
                float kk = m_cmykValues.w;

                m_color.r = (1.0f - cc) * (1.0f - kk);
                m_color.g = (1.0f - mm) * (1.0f - kk);
                m_color.b = (1.0f - yy) * (1.0f - kk);

                updateInputBoxes();
                if (m_onColorChanged) m_onColorChanged(m_color);
            });

            addChild(inputBox);
            m_cmykInputBoxes[i] = inputBox;
        }

        // hex box
        int initHex = ((int)(m_color.r * 255) << 16) | ((int)(m_color.g * 255) << 8) | (int)(m_color.b * 255);
        auto hexBox = std::make_shared<UIInputBox>(
            "HexInput",
            glm::vec2(8.0f, 150.0f),
            glm::vec2(popupWidth - 16.0f, 22.0f),
            (float)initHex
        );
        hexBox->setFontLoader(m_fontLoader);
        hexBox->setDrawsBackground(true);
        hexBox->setColor(glm::vec4(0.08f, 0.09f, 0.12f, 1.0f));

        hexBox->setOnValueChanged([this](float val) {
            int hexVal = static_cast<int>(val);
            m_color.r = ((hexVal >> 16) & 0xFF) / 255.0f;
            m_color.g = ((hexVal >> 8) & 0xFF) / 255.0f;
            m_color.b = (hexVal & 0xFF) / 255.0f;

            updateInputBoxes();
            if (m_onColorChanged) m_onColorChanged(m_color);
        });

        addChild(hexBox);
        m_hexInputBox = hexBox;
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
                if (m_isOpen) {
                    rebuildPopupElements();
                } else {
                    m_children.clear();
                }

                if (m_onLayoutChanged) {
                    m_onLayoutChanged();
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
            float popupHeight = 195.0f;
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
                m_fontLoader->generateTextGeometry("RGB", glm::vec2(popupPos.x + 8.0f, popupPos.y + 16.0f), glm::vec4(0.7f, 0.7f, 0.75f, 1.0f), vertices, indices);
                m_fontLoader->generateTextGeometry("CMYK", glm::vec2(popupPos.x + 8.0f, popupPos.y + 73.0f), glm::vec4(0.7f, 0.7f, 0.75f, 1.0f), vertices, indices);
                m_fontLoader->generateTextGeometry("HEX", glm::vec2(popupPos.x + 8.0f, popupPos.y + 128.0f), glm::vec4(0.7f, 0.7f, 0.75f, 1.0f), vertices, indices);
            }

            for (auto& child : m_children) {
                if (child) {
                    child->generateGeometry(vertices, indices);
                }
            }
        }
    }
}