#pragma once
#include "ui_element.hpp"
#include "ui/ui_input_box.hpp"
#include "ui/ui_button.hpp"
#include "resources/font_loader.hpp"
#include <functional>
#include <memory>
#include <glm/glm.hpp>

namespace slate {
    class UIInspectorPanel : public UIElement {
    public:
        UIInspectorPanel(const std::string& name, glm::vec2 position, glm::vec2 size);

        void setSize(glm::vec2 size) override;
        void buildDefaultLayout();
        void onEvent(const SDL_Event& event) override;
        void generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices) override;

        void setFontLoader(std::shared_ptr<FontLoader> fontLoader) { m_fontLoader = fontLoader; }
        void setTargetObject(const std::string& name) {/* blank stub for engine.cpp */}

        void setPositionValues(const glm::vec3& pos);
        void setRotationValues(const glm::vec3& rot);
        void setScaleValues(const glm::vec3& scl);

        void setMaterialColorValues(const glm::vec4& color);

        void setRoughness(float val);
        void setMetallic(float val);
        void setIOR(float val);
        void setTransmission(float val);

        float getMaterialFloatValue(int index) const {
            if (index >= 0 && index < 4) {
                return m_materialFloatValues[index];
            }
            return 0.0f;
        }

        void setOnPositionChanged(std::function<void(float, float, float)> cb) { m_onPositionChanged = cb; }
        void setOnRotationChanged(std::function<void(float, float, float)> cb) { m_onRotationChanged = cb; }
        void setOnScaleChanged(std::function<void(float, float, float)> cb) { m_onScaleChanged = cb; }

        void setOnMaterialVec4Changed(std::function<void(int, const glm::vec4&)> cb) { m_onMaterialVec4Changed = cb; }
        void setOnMaterialFloatChanged(std::function<void(int, float)> cb) { m_onMaterialFloatChanged = cb; }

    private:
        void updateChildLayouts();

        std::shared_ptr<FontLoader> m_fontLoader;

        glm::vec3 m_positionValues{0.0f};
        glm::vec3 m_rotationValues{0.0f};
        glm::vec3 m_scaleValues{1.0f};
        glm::vec4 m_materialColorValues{1.0f};
        float m_materialFloatValues[4]{0.5f, 0.0f, 1.5f, 0.0f};

        std::shared_ptr<UIInputBox> m_posInputBoxes[3];
        std::shared_ptr<UIInputBox> m_rotInputBoxes[3];
        std::shared_ptr<UIInputBox> m_sclInputBoxes[3];

        std::shared_ptr<UIInputBox> m_matColorInputBoxes[3];
        std::shared_ptr<UIInputBox> m_matFloatInputBoxes[4];

        std::function<void(float, float, float)> m_onPositionChanged;
        std::function<void(float, float, float)> m_onRotationChanged;
        std::function<void(float, float, float)> m_onScaleChanged;

        std::function<void(int, const glm::vec4&)> m_onMaterialVec4Changed;
        std::function<void(int, float)> m_onMaterialFloatChanged;
    };
}