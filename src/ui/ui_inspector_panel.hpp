#pragma once

#include "ui/ui_element.hpp"
#include "ui/ui_button.hpp"
#include "resources/font_loader.hpp"
#include <string>
#include <memory>
#include <functional>
#include <glm/vec3.hpp>

namespace slate {

    class UIInspectorPanel : public UIElement {
    public:
        UIInspectorPanel(const std::string& name, glm::vec2 position, glm::vec2 size);

        void setSize(glm::vec2 size) override;
        void setTargetObject(const std::string& objectName) { m_targetObjectName = objectName; }
        void setFontLoader(std::shared_ptr<FontLoader> fontLoader) { m_fontLoader = std::move(fontLoader); }
        void setOnTransformChanged(std::function<void(float x, float y, float z)> callback) { m_onPositionChanged = callback; }

        void buildDefaultLayout();
        void onEvent(const SDL_Event& event) override;

    private:
        void updateChildLayouts();

        std::string m_targetObjectName{"None"};
        std::shared_ptr<FontLoader> m_fontLoader{nullptr};
        std::function<void(float, float, float)> m_onPositionChanged;

        glm::vec3 m_positionValues{0.0f, 0.0f, 0.0f};
        glm::vec3 m_rotationValues{0.0f, 0.0f, 0.0f};
        glm::vec3 m_scaleValues{1.0f, 1.0f, 1.0f};
    };

}