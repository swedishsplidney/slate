#pragma once

#include "ui/ui_element.hpp"
#include "resources/font_loader.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace slate {
    
    class UIManager;

    struct MenuItem {
        std::string label;
        std::string commandId;
        std::string shortcut;
        bool isSeparator{false};
        std::vector<MenuItem> subItems;
    };

    struct MenuHeader {
        std::string title;
        std::vector<MenuItem> items;
    };

    class UIMenuBar : public UIElement {
    public:
        using CommandCallback = std::function<void(const std::string&)>;

        UIMenuBar(const std::string& name, glm::vec2 position, glm::vec2 size);

        void addMenu(MenuHeader menu);
        void setOnCommandTriggered(CommandCallback callback) { m_onCommand = std::move(callback); }

        void setUIManager(UIManager* uiManager) { m_uiManager = uiManager; }
        void setFontLoader(std::shared_ptr<FontLoader> fontLoader) { m_fontLoader = std::move(fontLoader); }

        void onEvent(const SDL_Event& event) override;
        void generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) override;

    private:
        std::vector<MenuHeader> m_menus;
        int m_activeMenuIndex{-1};
        CommandCallback m_onCommand{nullptr};
        UIManager* m_uiManager{nullptr};
        std::shared_ptr<FontLoader> m_fontLoader{nullptr};
    };

}