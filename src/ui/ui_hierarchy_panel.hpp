#pragma once
#include "ui/ui_element.hpp"
#include "resources/font_loader.hpp"
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace slate {
    class UIHierarchyPanel : public UIElement {
    public:
        UIHierarchyPanel(const std::string& name, glm::vec2 position, glm::vec2 size);

        void setSize(glm::vec2 size) override;
        void buildDefaultLayout();
        void onEvent(const SDL_Event& event) override;
        void generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) override;

        void setFontLoader(std::shared_ptr<FontLoader> fontLoader) { m_fontLoader = fontLoader; }
        void setSceneItems(const std::vector<std::string>& itemNames);
        void setSelectedIndex(int index) { m_selectedIndex = index; }

        void setOnItemSelected(std::function<void(int)> cb) { m_onItemSelected = cb; }

    private:
        std::shared_ptr<FontLoader> m_fontLoader;
        std::vector<std::string> m_itemNames;
        int m_selectedIndex = -1;
        std::function<void(int)> m_onItemSelected;
    };
}