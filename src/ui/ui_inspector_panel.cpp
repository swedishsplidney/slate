#include "ui_inspector_panel.hpp"
#include "ui/ui_input_box.hpp"
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
        headerBar->setDrawsBackground(false);
        headerBar->setColor(glm::vec4(0.032f, 0.036f, 0.046f, 1.0f));
        addChild(headerBar);

        auto transformSection = std::make_shared<UIElement>("TransformComponent", glm::vec2(8.0f, 36.0f), glm::vec2(panelWidth - 16.0f, 180.0f));
        transformSection->setDrawsBackground(false);
        transformSection->setColor(glm::vec4(0.016f, 0.018f, 0.023f, 1.0f));
        addChild(transformSection);

        float sectionWidth = transformSection->getSize().x;
        float fieldWidth = (sectionWidth - 65.0f) / 3.0f;
        float startX = 45.0f;

        auto createInputRow = [&](float rowY, float defaultVal, std::function<void(float, int)> callback, std::shared_ptr<UIInputBox> outBoxes[]) {
            for (int i = 0; i < 3; ++i) {
                float fx = startX + (i * (fieldWidth + 4.0f));
                auto inputBox = std::make_shared<UIInputBox>(
                    "InputBox_" + std::to_string(i),
                    glm::vec2(fx, rowY),
                    glm::vec2(fieldWidth, 22.0f),
                    defaultVal
                );
                inputBox->setFontLoader(m_fontLoader);
                inputBox->setOnValueChanged([callback, i](float val) {
                    callback(val, i);
                });
                transformSection->addChild(inputBox);

                if (outBoxes) {
                    outBoxes[i] = inputBox;
                }
            }
        };

        createInputRow(25.0f, 0.0f, [this](float val, int axis) {
            if (axis == 0) m_positionValues.x = val;
            else if (axis == 1) m_positionValues.y = val;
            else if (axis == 2) m_positionValues.z = val;

            if (m_onPositionChanged) {
                m_onPositionChanged(m_positionValues.x, m_positionValues.y, m_positionValues.z);
            }
        }, m_posInputBoxes);

        createInputRow(55.0f, 0.0f, [this](float val, int axis) {
            if (axis == 0) m_rotationValues.x = val;
            else if (axis == 1) m_rotationValues.y = val;
            else if (axis == 2) m_rotationValues.z = val;

            if (m_onRotationChanged) {
                m_onRotationChanged(m_rotationValues.x, m_rotationValues.y, m_rotationValues.z);
            }
        }, m_rotInputBoxes);

        createInputRow(85.0f, 1.0f, [this](float val, int axis) {
            if (axis == 0) m_scaleValues.x = val;
            else if (axis == 1) m_scaleValues.y = val;
            else if (axis == 2) m_scaleValues.z = val;

            if (m_onScaleChanged) {
                m_onScaleChanged(m_scaleValues.x, m_scaleValues.y, m_scaleValues.z);
            }
        }, m_sclInputBoxes);

        auto resetButton = std::make_shared<UIButton>(
            "ResetButton",
            glm::vec2(10.0f, 130.0f),
            glm::vec2(sectionWidth - 20.0f, 25.0f),
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

        auto drawBox = [&](const std::shared_ptr<UIElement>& element) {
            if (element && element->isVisible()) {
                glm::vec2 cPos = element->getAbsolutePosition();
                glm::vec2 cSize = element->getSize();
                glm::vec4 cColor = element->getColor();
                uint16_t cIdx = static_cast<uint16_t>(vertices.size());
                vertices.push_back(UIVertex{.pos = cPos, .color = cColor, .uv = glm::vec2(-1.0f)});
                vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x + cSize.x, cPos.y), .color = cColor, .uv = glm::vec2(-1.0f)});
                vertices.push_back(UIVertex{.pos = glm::vec2(cPos.x + cSize.x, cPos.y + cSize.y), .color = cColor, .uv = glm::vec2(-1.0f)});
                vertices.push_back(UIVertex{.pos = glm::vec2(abs(cPos.x), cPos.y + cSize.y), .color = cColor, .uv = glm::vec2(-1.0f)});
                indices.push_back(cIdx + 0); indices.push_back(cIdx + 1); indices.push_back(cIdx + 2);
                indices.push_back(cIdx + 0); indices.push_back(cIdx + 2); indices.push_back(cIdx + 3);
            }
        };

        drawBox(headerBar);
        drawBox(transformSection);
        drawBox(materialSection);

        if (!m_fontLoader) {
            std::cout << "[ui error] inspector panel has no fontloader! text generation skipped.\n";
            return;
        }

        glm::vec2 headerAbsPos = headerBar->getAbsolutePosition();
        glm::vec2 transAbsPos = transformSection->getAbsolutePosition();
        glm::vec2 matAbsPos = materialSection->getAbsolutePosition();

        glm::vec4 textColor(0.85f, 0.85f, 0.90f, 1.0f);
        glm::vec4 dimTextColor(0.55f, 0.55f, 0.60f, 1.0f);

        m_fontLoader->generateTextGeometry("Inspector", glm::vec2(headerAbsPos.x + 12.0f, headerAbsPos.y + 18.0f), textColor, vertices, indices);
        m_fontLoader->generateTextGeometry("Transform", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 16.0f), textColor, vertices, indices);

        m_fontLoader->generateTextGeometry("Pos", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 34.0f), dimTextColor, vertices, indices);
        m_fontLoader->generateTextGeometry("Rot", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 64.0f), dimTextColor, vertices, indices);
        m_fontLoader->generateTextGeometry("Scl", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 94.0f), dimTextColor, vertices, indices);

        m_fontLoader->generateTextGeometry("Materials", glm::vec2(matAbsPos.x + 12.0f, matAbsPos.y + 16.0f), textColor, vertices, indices);

        transformSection->generateGeometry(vertices, indices);
        materialSection->generateGeometry(vertices, indices);
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
        }
    }

    void UIInspectorPanel::setPositionValues(const glm::vec3& pos) {
        m_positionValues = pos;
        if (m_posInputBoxes[0]) m_posInputBoxes[0]->setValueWithoutCallback(pos.x);
        if (m_posInputBoxes[1]) m_posInputBoxes[1]->setValueWithoutCallback(pos.y);
        if (m_posInputBoxes[2]) m_posInputBoxes[2]->setValueWithoutCallback(pos.z);
    }

    void UIInspectorPanel::setRotationValues(const glm::vec3& rot) {
        m_rotationValues = rot;
        if (m_rotInputBoxes[0]) m_rotInputBoxes[0]->setValueWithoutCallback(rot.x);
        if (m_rotInputBoxes[1]) m_rotInputBoxes[1]->setValueWithoutCallback(rot.y);
        if (m_rotInputBoxes[2]) m_rotInputBoxes[2]->setValueWithoutCallback(rot.z);
    }

    void UIInspectorPanel::setScaleValues(const glm::vec3& scl) {
        m_scaleValues = scl;
        if (m_sclInputBoxes[0]) m_sclInputBoxes[0]->setValueWithoutCallback(scl.x);
        if (m_sclInputBoxes[1]) m_sclInputBoxes[1]->setValueWithoutCallback(scl.y);
        if (m_sclInputBoxes[2]) m_sclInputBoxes[2]->setValueWithoutCallback(scl.z);
    }

}