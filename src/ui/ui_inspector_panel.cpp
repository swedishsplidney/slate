#include "ui_inspector_panel.hpp"
#include <iostream>

namespace slate {

    UIInspectorPanel::UIInspectorPanel(const std::string& name, glm::vec2 position, glm::vec2 size)
        : UIElement(name, position, size) {
        setDrawsBackground(true);
        setColor(glm::vec4(0.13f, 0.14f, 0.18f, 1.0f));

        if (m_size.x <= 0.0f || m_size.y <= 0.0f) {
            m_size = glm::vec2(300.0f, 400.0f);
        }
    }

    void UIInspectorPanel::setSize(glm::vec2 size) {
        UIElement::setSize(size);
        updateChildLayouts();
    }

    void UIInspectorPanel::buildDefaultLayout() {
        float panelWidth = m_size.x > 0 ? m_size.x : 300.0f;

        auto headerBar = std::make_shared<UIElement>("InspectorHeader", glm::vec2(0.0f, 0.0f), glm::vec2(panelWidth, 28.0f));
        headerBar->setDrawsBackground(true);
        headerBar->setColor(glm::vec4(0.032f, 0.036f, 0.046f, 1.0f));
        addChild(headerBar);

        auto transformSection = std::make_shared<UIElement>("TransformComponent", glm::vec2(8.0f, 36.0f), glm::vec2(panelWidth - 16.0f, 180.0f));
        transformSection->setDrawsBackground(false);
        transformSection->setColor(glm::vec4(0.016f, 0.018f, 0.023f, 1.0f));
        addChild(transformSection);

        auto resetButton = std::make_shared<UIButton>(
            "ResetButton",
            glm::vec2(10.0f, 145.0f),
            glm::vec2(transformSection->getSize().x - 20.0f, 25.0f),
            [this]() {
                if (m_onPositionChanged) {
                    m_onPositionChanged(0.0f, 0.0f, 0.0f);
                }
            }
        );
        resetButton->setDrawsBackground(true);
        resetButton->setColor(glm::vec4(0.022f, 0.035f, 0.055f, 1.0f));
        transformSection->addChild(resetButton);

        auto materialSection = std::make_shared<UIElement>("MaterialComponent", glm::vec2(8.0f, 224.0f), glm::vec2(panelWidth - 16.0f, 120.0f));
        materialSection->setDrawsBackground(false);
        materialSection->setColor(glm::vec4(0.016f, 0.018f, 0.023f, 1.0f));
        addChild(materialSection);
    }

    void UIInspectorPanel::onEvent(const SDL_Event& event) {
        UIElement::onEvent(event);
    }

    void UIInspectorPanel::generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) {
        if (!m_visible) return;

        if (m_drawsBackground) {
            glm::vec2 absPos = getAbsolutePosition();
            uint16_t panelIdx = static_cast<uint16_t>(vertices.size());
            vertices.push_back(UIVertex{.pos = absPos, .color = m_color, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + m_size.x, absPos.y), .color = m_color, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + m_size.x, absPos.y + m_size.y), .color = m_color, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x, absPos.y + m_size.y), .color = m_color, .uv = glm::vec2(-1.0f)});
            indices.push_back(panelIdx + 0); indices.push_back(panelIdx + 1); indices.push_back(panelIdx + 2);
            indices.push_back(panelIdx + 0); indices.push_back(panelIdx + 2); indices.push_back(panelIdx + 3);
        }

        if (m_children.size() < 3) return;

        auto headerBar = m_children[0];
        auto transformSection = m_children[1];
        auto materialSection = m_children[2];

        if (headerBar && headerBar->isVisible()) {
            glm::vec2 cPos = headerBar->getAbsolutePosition();
            glm::vec2 cSize = headerBar->getSize();
            glm::vec4 cColor = headerBar->getColor();
            uint16_t cIdx = static_cast<uint16_t>(vertices.size());
            vertices.push_back(UIVertex{.pos = cPos, .color = cColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x + cSize.x, cPos.y), .color = cColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x + cSize.x, cPos.y + cSize.y), .color = cColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x, cPos.y + cSize.y), .color = cColor, .uv = glm::vec2(-1.0f)});
            indices.push_back(cIdx + 0); indices.push_back(cIdx + 1); indices.push_back(cIdx + 2);
            indices.push_back(cIdx + 0); indices.push_back(cIdx + 2); indices.push_back(cIdx + 3);
        }

        if (transformSection && transformSection->isVisible()) {
            glm::vec2 cPos = transformSection->getAbsolutePosition();
            glm::vec2 cSize = transformSection->getSize();
            glm::vec4 cColor = transformSection->getColor();
            uint16_t cIdx = static_cast<uint16_t>(vertices.size());
            vertices.push_back(UIVertex{.pos = cPos, .color = cColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x + cSize.x, cPos.y), .color = cColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x + cSize.x, cPos.y + cSize.y), .color = cColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x, cPos.y + cSize.y), .color = cColor, .uv = glm::vec2(-1.0f)});
            indices.push_back(cIdx + 0); indices.push_back(cIdx + 1); indices.push_back(cIdx + 2);
            indices.push_back(cIdx + 0); indices.push_back(cIdx + 2); indices.push_back(cIdx + 3);
        }

        if (materialSection && materialSection->isVisible()) {
            glm::vec2 cPos = materialSection->getAbsolutePosition();
            glm::vec2 cSize = materialSection->getSize();
            glm::vec4 cColor = materialSection->getColor();
            uint16_t cIdx = static_cast<uint16_t>(vertices.size());
            vertices.push_back(UIVertex{.pos = cPos, .color = cColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x + cSize.x, cPos.y), .color = cColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x + cSize.x, cPos.y + cSize.y), .color = cColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x, cPos.y + cSize.y), .color = cColor, .uv = glm::vec2(-1.0f)});
            indices.push_back(cIdx + 0); indices.push_back(cIdx + 1); indices.push_back(cIdx + 2);
            indices.push_back(cIdx + 0); indices.push_back(cIdx + 2); indices.push_back(cIdx + 3);
        }

        if (!m_fontLoader) {
            std::cout << "[ui error] inspector panel has no fontloader! text generation skipped.\n";
            return;
        }

        glm::vec2 transAbsPos = transformSection->getAbsolutePosition();
        glm::vec2 matAbsPos = materialSection->getAbsolutePosition();
        float sectionWidth = transformSection->getSize().x;

        glm::vec4 textColor(0.85f, 0.85f, 0.90f, 1.0f);
        glm::vec4 dimTextColor(0.55f, 0.55f, 0.60f, 1.0f);
        glm::vec4 inputFieldColor(0.032f, 0.036f, 0.046f, 1.0f);

        m_fontLoader->generateTextGeometry("Transform", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 16.0f), textColor, vertices, indices);
        m_fontLoader->generateTextGeometry("Pos", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 44.0f), dimTextColor, vertices, indices);
        m_fontLoader->generateTextGeometry("Rot", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 74.0f), dimTextColor, vertices, indices);
        m_fontLoader->generateTextGeometry("Scl", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 104.0f), dimTextColor, vertices, indices);

        float fieldWidth = (sectionWidth - 65.0f) / 3.0f;
        float startX = transAbsPos.x + 45.0f;

        for (int i = 0; i < 3; ++i) {
            float fx = startX + (i * (fieldWidth + 4.0f));
            float fy = transAbsPos.y + 40.0f;

            uint16_t bIdx = static_cast<uint16_t>(vertices.size());
            vertices.push_back(UIVertex{.pos = glm::vec2(fx, fy), .color = inputFieldColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(fx + fieldWidth, fy), .color = inputFieldColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(fx + fieldWidth, fy + 22.0f), .color = inputFieldColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(fx, fy + 22.0f), .color = inputFieldColor, .uv = glm::vec2(-1.0f)});

            indices.push_back(bIdx + 0); indices.push_back(bIdx + 1); indices.push_back(bIdx + 2);
            indices.push_back(bIdx + 0); indices.push_back(bIdx + 2); indices.push_back(bIdx + 3);

            std::string valStr = (i == 0) ? std::to_string(m_positionValues.x) : (i == 1) ? std::to_string(m_positionValues.y) : std::to_string(m_positionValues.z);
            if (valStr.length() > 5) valStr = valStr.substr(0, 5);

            m_fontLoader->generateTextGeometry(valStr, glm::vec2(fx + 6.0f, fy + 16.0f), textColor, vertices, indices);
        }

        m_fontLoader->generateTextGeometry("Materials", glm::vec2(matAbsPos.x + 12.0f, matAbsPos.y + 16.0f), textColor, vertices, indices);
    }

    void UIInspectorPanel::updateChildLayouts() {
        if (m_children.size() >= 3) {
            float panelWidth = m_size.x > 0 ? m_size.x : 300.0f;

            m_children[0]->setSize(glm::vec2(panelWidth, m_children[0]->getSize().y));

            auto transformSection = m_children[1];
            transformSection->setSize(glm::vec2(panelWidth - 16.0f, transformSection->getSize().y));

            auto materialSection = m_children[2];
            materialSection->setPosition(glm::vec2(8.0f, transformSection->getPosition().y + transformSection->getSize().y + 8.0f));
            materialSection->setSize(glm::vec2(panelWidth - 16.0f, materialSection->getSize().y));

            auto& innerChildren = transformSection->getChildren();
            if (!innerChildren.empty()) {
                innerChildren[0]->setSize(glm::vec2(transformSection->getSize().x - 20.0f, innerChildren[0]->getSize().y));
            }
        }
    }

}