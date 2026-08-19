#include "ui_inspector_panel.hpp"
#include <iostream>

namespace slate {

    UIInspectorPanel::UIInspectorPanel(const std::string& name, glm::vec2 position, glm::vec2 size)
        : UIElement(name, position, size) {
        setDrawsBackground(true);
        setColor(glm::vec4(0.13f, 0.14f, 0.18f, 1.0f));
    }

    void UIInspectorPanel::setSize(glm::vec2 size) {
        UIElement::setSize(size);
        updateChildLayouts();
    }

    void UIInspectorPanel::buildDefaultLayout() {
        auto headerBar = std::make_shared<UIElement>("InspectorHeader", glm::vec2(0.0f, 0.0f), glm::vec2(m_size.x, 28.0f));
        headerBar->setDrawsBackground(true);
        headerBar->setColor(glm::vec4(0.09f, 0.10f, 0.13f, 1.0f));
        addChild(headerBar);

        auto transformSection = std::make_shared<UIElement>("TransformComponent", glm::vec2(8.0f, 36.0f), glm::vec2(m_size.x - 16.0f, 180.0f));
        transformSection->setDrawsBackground(true);
        transformSection->setColor(glm::vec4(0.16f, 0.18f, 0.23f, 1.0f));
        addChild(transformSection);

        auto resetButton = std::make_shared<UIButton>(
            "ResetButton",
            glm::vec2(10.0f, 140.0f),
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

        auto materialSection = std::make_shared<UIElement>("MaterialComponent", glm::vec2(8.0f, 224.0f), glm::vec2(m_size.x - 16.0f, 120.0f));
        materialSection->setDrawsBackground(true);
        materialSection->setColor(glm::vec4(0.16f, 0.18f, 0.23f, 1.0f));
        addChild(materialSection);
    }

    void UIInspectorPanel::onEvent(const SDL_Event& event) {
        UIElement::onEvent(event);
    }

    void UIInspectorPanel::updateChildLayouts() {
        if (m_children.size() >= 3) {
            m_children[0]->setSize(glm::vec2(m_size.x, m_children[0]->getSize().y));

            auto transformSection = m_children[1];
            transformSection->setSize(glm::vec2(m_size.x - 16.0f, transformSection->getSize().y));

            auto materialSection = m_children[2];
            materialSection->setPosition(glm::vec2(8.0f, transformSection->getSize().y + 44.0f));
            materialSection->setSize(glm::vec2(m_size.x - 16.0f, materialSection->getSize().y));

            auto& innerChildren = transformSection->getChildren();
            if (!innerChildren.empty()) {
                innerChildren[0]->setSize(glm::vec2(transformSection->getSize().x - 20.0f, innerChildren[0]->getSize().y));
            }
        }
    }

}