#include "ui_menu_bar.hpp"
#include "ui_manager.hpp"
#include <iostream>

namespace slate {

    UIMenuBar::UIMenuBar(const std::string& name, glm::vec2 position, glm::vec2 size)
        : UIElement(name, position, size) {
        setDrawsBackground(true);
        setColor(glm::vec4(0.010f, 0.011f, 0.013f, 1.0f));
    }

    void UIMenuBar::addMenu(MenuHeader menu) {
        m_menus.push_back(std::move(menu));
    }

    void UIMenuBar::onEvent(const SDL_Event& event) {
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
            glm::vec2 mousePos{event.button.x, event.button.y};
            glm::vec2 absPos = getAbsolutePosition();

            float currentX = absPos.x + 15.0f;
            float padding = 20.0f;
            bool stateChanged = false;

            for (size_t i = 0; i < m_menus.size(); ++i) {
                float itemWidth = (m_menus[i].title.length() * 8.0f) + padding;

                if (mousePos.x >= currentX - 5.0f && mousePos.x <= currentX + itemWidth - 5.0f &&
                    mousePos.y >= absPos.y && mousePos.y <= absPos.y + m_size.y) {

                    m_activeMenuIndex = (m_activeMenuIndex == static_cast<int>(i)) ? -1 : static_cast<int>(i);
                    stateChanged = true;
                    break;
                }
                currentX += itemWidth;
            }

            if (!stateChanged && m_activeMenuIndex >= 0) {
                float dropdownX = absPos.x + 15.0f;
                for (int k = 0; k < m_activeMenuIndex; ++k) {
                    dropdownX += (m_menus[k].title.length() * 8.0f) + padding;
                }

                float dropdownY = absPos.y + m_size.y;
                float dropdownWidth = 180.0f;
                float itemHeight = 25.0f;

                const auto& items = m_menus[m_activeMenuIndex].items;
                for (size_t j = 0; j < items.size(); ++j) {
                    float itemY = dropdownY + (j * itemHeight);
                    if (mousePos.x >= dropdownX && mousePos.x <= dropdownX + dropdownWidth &&
                        mousePos.y >= itemY && mousePos.y <= itemY + itemHeight) {

                        const auto& item = items[j];
                        if (!item.isSeparator && !item.commandId.empty() && m_onCommand) {
                            m_onCommand(item.commandId);
                        }
                        m_activeMenuIndex = -1;
                        stateChanged = true;
                        break;
                    }
                }

                if (!stateChanged) {
                    m_activeMenuIndex = -1;
                    stateChanged = true;
                }
            }

            if (stateChanged && m_uiManager) {
                m_uiManager->markDirty();
            }
        }
    }

    void UIMenuBar::generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) {
        if (!m_visible) return;

        UIElement::generateGeometry(vertices, indices);

        glm::vec2 absPos = getAbsolutePosition();
        float currentX = absPos.x + 15.0f;
        float padding = 20.0f;
        glm::vec4 textColor(0.25f, 0.25f, 0.28f, 1.0f);

        for (size_t i = 0; i < m_menus.size(); ++i) {
            if (m_fontLoader) {
                glm::vec2 textPos{currentX, absPos.y + (m_size.y * 0.65f)};
                m_fontLoader->generateTextGeometry(m_menus[i].title, textPos, textColor, vertices, indices);
            }
            currentX += (m_menus[i].title.length() * 8.0f) + padding;
        }

        if (m_activeMenuIndex >= 0 && m_activeMenuIndex < static_cast<int>(m_menus.size())) {
            float dropdownX = absPos.x + 10.0f;
            for (int k = 0; k < m_activeMenuIndex; ++k) {
                dropdownX += (m_menus[k].title.length() * 8.0f) + padding;
            }

            float dropdownY = absPos.y + m_size.y;
            float dropdownWidth = 180.0f;
            float dropdownHeight = m_menus[m_activeMenuIndex].items.size() * 25.0f;

            uint16_t baseIndex = static_cast<uint16_t>(vertices.size());
            glm::vec4 popupColor(0.025f, 0.025f, 0.028f, 0.95f);

            vertices.push_back(UIVertex{.pos = glm::vec2(dropdownX, dropdownY), .color = popupColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(dropdownX + dropdownWidth, dropdownY), .color = popupColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(dropdownX + dropdownWidth, dropdownY + dropdownHeight), .color = popupColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(dropdownX, dropdownY + dropdownHeight), .color = popupColor, .uv = glm::vec2(-1.0f)});

            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 3);

            if (m_fontLoader) {
                const auto& items = m_menus[m_activeMenuIndex].items;
                for (size_t j = 0; j < items.size(); ++j) {
                    if (items[j].isSeparator) continue;

                    glm::vec2 itemTextPos{dropdownX + 10.0f, dropdownY + 18.0f + (j * 25.0f)};
                    m_fontLoader->generateTextGeometry(items[j].label, itemTextPos, textColor, vertices, indices);
                }
            }
        }
    }

}