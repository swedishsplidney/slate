#include "ui_inspector_panel.hpp"
#include "ui/ui_input_box.hpp"
#include <iostream>
#include <algorithm>

namespace slate {

    UIInspectorPanel::UIInspectorPanel(const std::string& name, glm::vec2 position, glm::vec2 size)
        : UIElement(name, position, size) {
        setDrawsBackground(true);
        setColor(glm::vec4(0.13f, 0.14f, 0.18f, 1.0f));

        if (m_size.x <= 0.0f || m_size.y <= 0.0f) {
            m_size = glm::vec2(300.0f, 680.0f);
        }
    }

    void UIInspectorPanel::setSize(glm::vec2 size) {
        UIElement::setSize(size);
        updateChildLayouts();
    }

    void UIInspectorPanel::buildDefaultLayout() {
        float panelWidth = m_size.x > 0 ? m_size.x : 300.0f;

        // title bar
        auto headerBar = std::make_shared<UIElement>("InspectorHeader", glm::vec2(0.0f, 0.0f), glm::vec2(panelWidth, 28.0f));
        headerBar->setDrawsBackground(false);
        headerBar->setColor(glm::vec4(0.016f, 0.018f, 0.023f, 1.0f));
        addChild(headerBar);

        auto transformDropdown = std::make_shared<UIDropdown>(
            "TransformDropdown",
            glm::vec2(8.0f, 36.0f),
            glm::vec2(panelWidth - 16.0f, 190.0f),
            "Transform",
            true
        );
        transformDropdown->setFontLoader(m_fontLoader);
        transformDropdown->setDrawsBackground(true);
        transformDropdown->setColor(glm::vec4(0.10f, 0.11f, 0.15f, 1.0f));
        transformDropdown->setOnToggle([this](bool) { updateChildLayouts(); });
        addChild(transformDropdown);

        float sectionWidth = transformDropdown->getSize().x;
        float fieldWidth = (sectionWidth - 68.0f) / 3.0f;
        float startX = 48.0f;

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
                inputBox->setDrawsBackground(true);
                inputBox->setColor(glm::vec4(0.08f, 0.09f, 0.12f, 1.0f));

                std::weak_ptr<UIInputBox> weakBox = inputBox;
                inputBox->setOnValueChanged([callback, i, weakBox](float val) {
                    callback(val, i);
                });
                transformDropdown->addContentElement(inputBox);

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

        createInputRow(75.0f, 0.0f, [this](float val, int axis) {
            if (axis == 0) m_rotationValues.x = val;
            else if (axis == 1) m_rotationValues.y = val;
            else if (axis == 2) m_rotationValues.z = val;

            if (m_onRotationChanged) {
                m_onRotationChanged(m_rotationValues.x, m_rotationValues.y, m_rotationValues.z);
            }
        }, m_rotInputBoxes);

        createInputRow(125.0f, 1.0f, [this](float val, int axis) {
            if (axis == 0) m_scaleValues.x = val;
            else if (axis == 1) m_scaleValues.y = val;
            else if (axis == 2) m_scaleValues.z = val;

            if (m_onScaleChanged) {
                m_onScaleChanged(m_scaleValues.x, m_scaleValues.y, m_scaleValues.z);
            }
        }, m_sclInputBoxes);

        auto materialDropdown = std::make_shared<UIDropdown>(
            "MaterialDropdown",
            glm::vec2(8.0f, 240.0f),
            glm::vec2(panelWidth - 16.0f, 290.0f),
            "Material",
            true
        );
        materialDropdown->setFontLoader(m_fontLoader);
        materialDropdown->setDrawsBackground(true);
        materialDropdown->setColor(glm::vec4(0.10f, 0.11f, 0.15f, 1.0f));
        materialDropdown->setOnToggle([this](bool) { updateChildLayouts(); });
        addChild(materialDropdown);

        auto createMatInputRow = [&](float rowY, float defaultVal, std::function<void(float, int)> callback, std::shared_ptr<UIInputBox> outBoxes[]) {
            for (int i = 0; i < 3; ++i) {
                float fx = startX + (i * (fieldWidth + 4.0f));
                auto inputBox = std::make_shared<UIInputBox>(
                    "MatInputBox_" + std::to_string(i),
                    glm::vec2(fx, rowY),
                    glm::vec2(fieldWidth, 22.0f),
                    defaultVal
                );
                inputBox->setFontLoader(m_fontLoader);
                inputBox->setDrawsBackground(true);
                inputBox->setColor(glm::vec4(0.08f, 0.09f, 0.12f, 1.0f));

                std::weak_ptr<UIInputBox> weakBox = inputBox;
                inputBox->setOnValueChanged([callback, i, weakBox](float val) {
                    float clamped = std::clamp(val, 0.0f, 1.0f);
                    if (clamped != val) {
                        if (auto box = weakBox.lock()) {
                            box->setValueWithoutCallback(clamped);
                        }
                    }
                    callback(clamped, i);
                });
                materialDropdown->addContentElement(inputBox);

                if (outBoxes) {
                    outBoxes[i] = inputBox;
                }
            }
        };

        createMatInputRow(25.0f, 1.0f, [this](float val, int idx) {
            if (idx == 0) m_materialColorValues.r = val;
            else if (idx == 1) m_materialColorValues.g = val;
            else if (idx == 2) m_materialColorValues.b = val;
            m_materialColorValues.a = 1.0f;

            if (m_onMaterialVec4Changed) {
                m_onMaterialVec4Changed(0, m_materialColorValues);
            }
        }, m_matColorInputBoxes);

        float fullFieldX = 50.0f;
        float fullFieldWidth = sectionWidth - 60.0f;

        float floatRowY[4] = { 75.0f, 125.0f, 175.0f, 225.0f };
        float defaultFloatVals[4] = { 0.5f, 0.0f, 1.5f, 0.0f };

        for (int i = 0; i < 4; ++i) {
            auto inputBox = std::make_shared<UIInputBox>(
                "MatFloatBox_" + std::to_string(i),
                glm::vec2(fullFieldX, floatRowY[i]),
                glm::vec2(fullFieldWidth, 22.0f),
                defaultFloatVals[i]
            );
            inputBox->setFontLoader(m_fontLoader);
            inputBox->setDrawsBackground(true);
            inputBox->setColor(glm::vec4(0.08f, 0.09f, 0.12f, 1.0f));

            std::weak_ptr<UIInputBox> weakBox = inputBox;
            inputBox->setOnValueChanged([this, i, weakBox](float val) {
                float clamped = val;
                if (i == 0 || i == 1 || i == 3) {
                    clamped = std::clamp(val, 0.0f, 1.0f);
                } else if (i == 2) {
                    clamped = std::max(1.0f, val);
                }

                if (clamped != val) {
                    if (auto box = weakBox.lock()) {
                        box->setValueWithoutCallback(clamped);
                    }
                }

                m_materialFloatValues[i] = clamped;

                int gpuIndex = i;
                switch (i) {
                    case 0: gpuIndex = 1; break;
                    case 1: gpuIndex = 2; break;
                    case 2: gpuIndex = 4; break;
                    case 3: gpuIndex = 3; break;
                }

                if (m_onMaterialFloatChanged) {
                    m_onMaterialFloatChanged(gpuIndex, clamped);
                }
            });
            materialDropdown->addContentElement(inputBox);
            m_matFloatInputBoxes[i] = inputBox;
        }

        updateChildLayouts();
    }

    void UIInspectorPanel::onEvent(const SDL_Event& event) {
        UIElement::onEvent(event);
    }

    void UIInspectorPanel::generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) {
        if (!m_visible) return;

        if (m_children.size() < 3) return;

        auto headerBar = m_children[0];
        auto transformDropdown = std::static_pointer_cast<UIDropdown>(m_children[1]);
        auto materialDropdown = std::static_pointer_cast<UIDropdown>(m_children[2]);

        if (m_drawsBackground) {
            glm::vec2 absPos = getAbsolutePosition();
            float panelWidth = m_size.x > 0 ? m_size.x : 300.0f;

            float transH = transformDropdown->isExpanded() ? transformDropdown->getSize().y : transformDropdown->getHeaderHeight();
            float matH = materialDropdown->isExpanded() ? materialDropdown->getSize().y : materialDropdown->getHeaderHeight();
            float matPosY = transformDropdown->getPosition().y + transH + 8.0f;
            float totalHeight = matPosY + matH + 10.0f;

            glm::vec4 panelBgColor(0.004f, 0.0045f, 0.0055f, 1.0f);

            uint16_t panelIdx = static_cast<uint16_t>(vertices.size());
            vertices.push_back(UIVertex{.pos = absPos, .color = panelBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + panelWidth, absPos.y), .color = panelBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x + panelWidth, absPos.y + totalHeight), .color = panelBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(absPos.x, absPos.y + totalHeight), .color = panelBgColor, .uv = glm::vec2(-1.0f)});
            indices.push_back(panelIdx + 0); indices.push_back(panelIdx + 1); indices.push_back(panelIdx + 2);
            indices.push_back(panelIdx + 0); indices.push_back(panelIdx + 2); indices.push_back(panelIdx + 3);
        }

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

        if (!m_fontLoader) {
            std::cout << "[ui error] inspector panel has no fontloader! text generation skipped.\n";
            return;
        }

        glm::vec2 headerAbsPos = headerBar->getAbsolutePosition();
        glm::vec4 textColor(0.85f, 0.85f, 0.90f, 1.0f);
        glm::vec4 dimTextColor(0.55f, 0.55f, 0.60f, 1.0f);

        m_fontLoader->generateTextGeometry("Inspector", glm::vec2(headerAbsPos.x + 10.0f, headerAbsPos.y + 18.0f), textColor, vertices, indices);

        glm::vec4 dropdownBgColor(0.010f, 0.011f, 0.015f, 1.0f);

        if (transformDropdown->isExpanded()) {
            glm::vec2 dPos = transformDropdown->getAbsolutePosition();
            glm::vec2 dSize = transformDropdown->getSize();
            float headerH = transformDropdown->getHeaderHeight();
            uint16_t bgIdx = static_cast<uint16_t>(vertices.size());
            vertices.push_back(UIVertex{.pos = glm::vec2(dPos.x, dPos.y + headerH), .color = dropdownBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(dPos.x + dSize.x, dPos.y + headerH), .color = dropdownBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(dPos.x + dSize.x, dPos.y + dSize.y), .color = dropdownBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(dPos.x, dPos.y + dSize.y), .color = dropdownBgColor, .uv = glm::vec2(-1.0f)});
            indices.push_back(bgIdx + 0); indices.push_back(bgIdx + 1); indices.push_back(bgIdx + 2);
            indices.push_back(bgIdx + 0); indices.push_back(bgIdx + 2); indices.push_back(bgIdx + 3);
        }

        if (materialDropdown->isExpanded()) {
            glm::vec2 dPos = materialDropdown->getAbsolutePosition();
            glm::vec2 dSize = materialDropdown->getSize();
            float headerH = materialDropdown->getHeaderHeight();
            uint16_t bgIdx = static_cast<uint16_t>(vertices.size());
            vertices.push_back(UIVertex{.pos = glm::vec2(dPos.x, dPos.y + headerH), .color = dropdownBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(dPos.x + dSize.x, dPos.y + headerH), .color = dropdownBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(dPos.x + dSize.x, dPos.y + dSize.y), .color = dropdownBgColor, .uv = glm::vec2(-1.0f)});
            vertices.push_back(UIVertex{.pos = glm::vec2(dPos.x, dPos.y + dSize.y), .color = dropdownBgColor, .uv = glm::vec2(-1.0f)});
            indices.push_back(bgIdx + 0); indices.push_back(bgIdx + 1); indices.push_back(bgIdx + 2);
            indices.push_back(bgIdx + 0); indices.push_back(bgIdx + 2); indices.push_back(bgIdx + 3);
        }

        // generate dropdown geometry
        transformDropdown->generateGeometry(vertices, indices);
        materialDropdown->generateGeometry(vertices, indices);

        glm::vec2 transAbsPos = transformDropdown->getAbsolutePosition();
        if (transformDropdown->isExpanded()) {
            m_fontLoader->generateTextGeometry("Position", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 28.0f + 19.0f), dimTextColor, vertices, indices);
            m_fontLoader->generateTextGeometry("Rotation", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 28.0f + 69.0f), dimTextColor, vertices, indices);
            m_fontLoader->generateTextGeometry("Scale", glm::vec2(transAbsPos.x + 12.0f, transAbsPos.y + 28.0f + 119.0f), dimTextColor, vertices, indices);
        }

        glm::vec2 matAbsPos = materialDropdown->getAbsolutePosition();
        if (materialDropdown->isExpanded()) {
            m_fontLoader->generateTextGeometry("Color", glm::vec2(matAbsPos.x + 12.0f, matAbsPos.y + 28.0f + 19.0f), dimTextColor, vertices, indices);
            m_fontLoader->generateTextGeometry("Roughness", glm::vec2(matAbsPos.x + 12.0f, matAbsPos.y + 28.0f + 69.0f), dimTextColor, vertices, indices);
            m_fontLoader->generateTextGeometry("Metallic", glm::vec2(matAbsPos.x + 12.0f, matAbsPos.y + 28.0f + 119.0f), dimTextColor, vertices, indices);
            m_fontLoader->generateTextGeometry("IOR", glm::vec2(matAbsPos.x + 12.0f, matAbsPos.y + 28.0f + 169.0f), dimTextColor, vertices, indices);
            m_fontLoader->generateTextGeometry("Transmission", glm::vec2(matAbsPos.x + 12.0f, matAbsPos.y + 28.0f + 219.0f), dimTextColor, vertices, indices);
        }
    }

    void UIInspectorPanel::updateChildLayouts() {
        if (m_children.size() >= 3) {
            float panelWidth = m_size.x > 0 ? m_size.x : 300.0f;

            m_children[0]->setSize(glm::vec2(panelWidth, m_children[0]->getSize().y));

            auto transformDropdown = std::static_pointer_cast<UIDropdown>(m_children[1]);
            auto materialDropdown = std::static_pointer_cast<UIDropdown>(m_children[2]);

            transformDropdown->setSize(glm::vec2(panelWidth - 16.0f, transformDropdown->getSize().y));

            float transformHeight = transformDropdown->isExpanded() ? transformDropdown->getSize().y : transformDropdown->getHeaderHeight();
            float materialPosY = transformDropdown->getPosition().y + transformHeight + 8.0f;

            materialDropdown->setPosition(glm::vec2(8.0f, materialPosY));
            materialDropdown->setSize(glm::vec2(panelWidth - 16.0f, materialDropdown->getSize().y));

            float materialHeight = materialDropdown->isExpanded() ? materialDropdown->getSize().y : materialDropdown->getHeaderHeight();
            m_size.y = materialPosY + materialHeight + 10.0f;
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

    void UIInspectorPanel::setMaterialColorValues(const glm::vec4& color) {
        m_materialColorValues = color;
        if (m_matColorInputBoxes[0]) m_matColorInputBoxes[0]->setValueWithoutCallback(color.r);
        if (m_matColorInputBoxes[1]) m_matColorInputBoxes[1]->setValueWithoutCallback(color.g);
        if (m_matColorInputBoxes[2]) m_matColorInputBoxes[2]->setValueWithoutCallback(color.b);
    }

    void UIInspectorPanel::setRoughness(float val) {
        m_materialFloatValues[0] = val;
        if (m_matFloatInputBoxes[0]) {
            m_matFloatInputBoxes[0]->setValueWithoutCallback(val);
        }
    }

    void UIInspectorPanel::setMetallic(float val) {
        m_materialFloatValues[1] = val;
        if (m_matFloatInputBoxes[1]) {
            m_matFloatInputBoxes[1]->setValueWithoutCallback(val);
        }
    }

    void UIInspectorPanel::setIOR(float val) {
        m_materialFloatValues[2] = val;
        if (m_matFloatInputBoxes[2]) {
            m_matFloatInputBoxes[2]->setValueWithoutCallback(val);
        }
    }

    void UIInspectorPanel::setTransmission(float val) {
        m_materialFloatValues[3] = val;
        if (m_matFloatInputBoxes[3]) {
            m_matFloatInputBoxes[3]->setValueWithoutCallback(val);
        }
    }

}