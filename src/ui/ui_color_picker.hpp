#pragma once
#include "ui/ui_element.hpp"
#include "ui/ui_input_box.hpp"
#include "resources/font_loader.hpp"
#include <functional>
#include <memory>
#include <glm/glm.hpp>

namespace slate {
    class UIColorPicker : public UIElement {
    public:
        UIColorPicker(const std::string& name, glm::vec2 position, glm::vec2 size);

        void onEvent(const SDL_Event& event) override;
        void generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) override;

        void setColorValue(const glm::vec4& color);
        const glm::vec4& getColorValue() const { return m_color; }

        bool isOpen() const { return m_isOpen; }
        float getExpandedHeight() const;

        void setFontLoader(std::shared_ptr<FontLoader> fontLoader) { m_fontLoader = fontLoader; }
        void setOnColorChanged(std::function<void(const glm::vec4&)> cb) { m_onColorChanged = cb; }
        void setOnLayoutChanged(std::function<void()> cb) { m_onLayoutChanged = cb; }

    private:
        void rebuildPopupElements();
        void updateInputBoxes();

        glm::vec4 m_color{1.0f};
        glm::vec4 m_cmykValues{0.0f};
        bool m_isOpen = false;

        std::shared_ptr<FontLoader> m_fontLoader;
        std::shared_ptr<UIInputBox> m_rgbInputBoxes[3];
        std::shared_ptr<UIInputBox> m_cmykInputBoxes[4];
        std::shared_ptr<UIInputBox> m_hexInputBox;

        std::function<void(const glm::vec4&)> m_onColorChanged;
        std::function<void()> m_onLayoutChanged;
    };
}