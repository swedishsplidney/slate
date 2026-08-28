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

            float leftSideWidth = 0.0f;
            float rightSideWidth = 0.0f;

            std::vector<std::shared_ptr<UIElement>> leftChildren;
            std::vector<std::shared_ptr<UIElement>> rightChildren;

            for (auto& child : m_children) {
                auto it = m_dockMap.find(child);
                if (it == m_dockMap.end()) continue;
                if (it->second.slot == DockSlot::LeftSide) {
                    leftChildren.push_back(child);
                    leftSideWidth = std::max(leftSideWidth, it->second.preferredSize);
                } else if (it->second.slot == DockSlot::RightSide) {
                    rightChildren.push_back(child);
                    rightSideWidth = std::max(rightSideWidth, it->second.preferredSize);
                }
            }

            // top and bottom
            for (auto& child : m_children) {
                auto it = m_dockMap.find(child);
                if (it == m_dockMap.end()) continue;
                if (it->second.slot == DockSlot::TopBar) {
                    child->setPosition({0.0f, currentTop});
                    child->setSize({m_size.x, it->second.preferredSize});
                    currentTop += it->second.preferredSize;
                } else if (it->second.slot == DockSlot::BottomBar) {
                    currentBottom -= it->second.preferredSize;
                    child->setPosition({0.0f, currentBottom});
                    child->setSize({m_size.x, it->second.preferredSize});
                }
            }

            float availableHeight = currentBottom - currentTop;

            // left
            float currentLeft = 0.0f;
            if (!leftChildren.empty()) {
                for (auto& child : leftChildren) {
                    child->setPosition({currentLeft, currentTop});
                    child->setSize({leftSideWidth, availableHeight});
                }
                currentLeft += leftSideWidth;
            }

            // right
            float currentRight = m_size.x;
            if (!rightChildren.empty()) {
                currentRight -= rightSideWidth;
                for (auto& child : rightChildren) {
                    child->setPosition({currentRight, currentTop});
                    child->setSize({rightSideWidth, availableHeight});
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