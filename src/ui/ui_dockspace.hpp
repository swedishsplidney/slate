#pragma once

#include "ui/ui_element.hpp"
#include <unordered_map>

namespace slate {

    enum class DockSlot {
        None,
        TopBar,
        BottomBar,
        LeftSide,
        RightSide,
        Center
    };

    class UIDockSpace : public UIElement {
    public:
        UIDockSpace(const std::string& name, glm::vec2 position, glm::vec2 size)
            : UIElement(name, position, size) {}

        void setSize(glm::vec2 size) override {
            UIElement::setSize(size);
            recalculateLayout();
        }

        void addDockedChild(std::shared_ptr<UIElement> child, DockSlot slot, float preferredSize) {
            addChild(child);
            m_dockMap[child] = { slot, preferredSize };
            recalculateLayout();
        }

        void recalculateLayout() {
            float currentTop = 0.0f;
            float currentBottom = m_size.y;
            float currentLeft = 0.0f;
            float currentRight = m_size.x;

            // top and bottom bars
            for (auto& child : m_children) {
                auto it = m_dockMap.find(child);
                if (it == m_dockMap.end()) continue;

                DockSlot slot = it->second.slot;
                float prefSize = it->second.preferredSize;

                if (slot == DockSlot::TopBar) {
                    child->setPosition({0.0f, currentTop});
                    child->setSize({m_size.x, prefSize});
                    currentTop += prefSize;
                } else if (slot == DockSlot::BottomBar) {
                    currentBottom -= prefSize;
                    child->setPosition({0.0f, currentBottom});
                    child->setSize({m_size.x, prefSize});
                }
            }

            float availableHeight = currentBottom - currentTop;

            // side bars and center
            for (auto& child : m_children) {
                auto it = m_dockMap.find(child);
                if (it == m_dockMap.end()) continue;

                DockSlot slot = it->second.slot;
                float prefSize = it->second.preferredSize;

                if (slot == DockSlot::LeftSide) {
                    child->setPosition({currentLeft, currentTop});
                    child->setSize({prefSize, availableHeight});
                    currentLeft += prefSize;
                } else if (slot == DockSlot::RightSide) {
                    currentRight -= prefSize;
                    child->setPosition({currentRight, currentTop});
                    child->setSize({prefSize, availableHeight});
                }
            }

            // viewport
            for (auto& child : m_children) {
                auto it = m_dockMap.find(child);
                if (it == m_dockMap.end() || it->second.slot != DockSlot::Center) continue;

                child->setPosition({currentLeft, currentTop});
                child->setSize({currentRight - currentLeft, availableHeight});
            }
        }

    private:
        struct DockInfo {
            DockSlot slot;
            float preferredSize;
        };

        std::unordered_map<std::shared_ptr<UIElement>, DockInfo> m_dockMap;
    };

}