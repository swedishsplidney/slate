#pragma once

#include "ui/ui_element.hpp"
#include "resources/font_loader.hpp"
#include <functional>
#include <string>
#include <memory>

namespace slate {

    class UIDropdown : public UIElement {
    public:
        UIDropdown(const std::string& name, glm::vec2 position, glm::vec2 size, const std::string& title, bool defaultExpanded = true);

        void setFontLoader(std::shared_ptr<FontLoader> fontLoader) { m_fontLoader = fontLoader; }

        void update(float deltaTime) override;
        void onEvent(const SDL_Event& event) override;
        void generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t> &indices) override;

        bool isExpanded() const { return m_isExpanded; }
        void setExpanded(bool expanded);

        void setOnToggle(std::function<void(bool)> callback) { m_onToggleCallback = callback; }

        void addContentElement(const std::shared_ptr<UIElement>& element);

        float getHeaderHeight() const { return m_headerHeight; }

    private:
        std::string m_title;
        bool m_isExpanded;
        float m_headerHeight{28.0f};

        uint64_t m_lastClickTimestamp{0};

        std::shared_ptr<FontLoader> m_fontLoader;
        std::shared_ptr<UIElement> m_contentContainer;
        std::function<void(bool)> m_onToggleCallback;
    };
}