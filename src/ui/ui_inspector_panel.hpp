#pragma once

#include "ui/ui_element.hpp"
#include "ui/ui_button.hpp"
#include <string>
#include <memory>
#include <functional>

namespace slate {

    class UIInspectorPanel : public UIElement {
    public:
        UIInspectorPanel(const std::string& name, glm::vec2 position, glm::vec2 size)
            : UIElement(name, position, size) {

            setDrawsBackground(true);
            setColor(glm::vec4(0.13f, 0.14f, 0.18f, 1.0f));
        }

        void setSize(glm::vec2 size) override {
            UIElement::setSize(size);
            updateChildLayouts();
        }

        void setTargetObject(const std::string& objectName) {
            m_targetObjectName = objectName;
        }

        void setOnTransformChanged(std::function<void(float x, float y, float z)> callback) {
            m_onPositionChanged = callback;
        }

        void buildDefaultLayout() {
            auto headerBar = std::make_shared<UIElement>("InspectorHeader", glm::vec2(0.0f, 0.0f), glm::vec2(m_size.x, 28.0f));
            headerBar->setDrawsBackground(true);
            headerBar->setColor(glm::vec4(0.09f, 0.10f, 0.13f, 1.0f));
            addChild(headerBar);

            auto transformSection = std::make_shared<UIElement>("TransformComponent", glm::vec2(8.0f, 36.0f), glm::vec2(m_size.x - 16.0f, 140.0f));
            transformSection->setDrawsBackground(true);
            transformSection->setColor(glm::vec4(0.16f, 0.18f, 0.23f, 1.0f));
            addChild(transformSection);

            auto resetButton = std::make_shared<UIButton>(
                "ResetButton",
                glm::vec2(10.0f, 100.0f),
                glm::vec2(transformSection->getSize().x - 20.0f, 25.0f),
                [this]() {
                    if (m_onPositionChanged) {
                        m_onPositionChanged(0.0f, 0.0f, 0.0f);
                    }
                }
            );
            resetButton->setDrawsBackground(true);
            resetButton->setColor(glm::vec4(0.22f, 0.35f, 0.55f, 1.0f));
            transformSection->addChild(resetButton);
        }

    private:
        void updateChildLayouts() {
            if (m_children.size() >= 2) {
                m_children[0]->setSize(glm::vec2(m_size.x, m_children[0]->getSize().y));

                auto transformSection = m_children[1];
                glm::vec2 currentTransformSize = transformSection->getSize();
                transformSection->setSize(glm::vec2(m_size.x - 16.0f, currentTransformSize.y));

                auto& innerChildren = transformSection->getChildren();
                if (!innerChildren.empty()) {
                    innerChildren[0]->setSize(glm::vec2(transformSection->getSize().x - 20.0f, innerChildren[0]->getSize().y));
                }
            }
        }

        std::string m_targetObjectName{"None"};
        std::function<void(float, float, float)> m_onPositionChanged;
    };

}