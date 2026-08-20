#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <string>
#include <memory>
#include <cstdint> // <-- For uint16_t
#include <SDL3/SDL_events.h>
#include "ui/ui_vertex.hpp"

namespace slate {

    class UIElement : public std::enable_shared_from_this<UIElement> {
    public:
        UIElement(const std::string& name, glm::vec2 position, glm::vec2 size);
        virtual ~UIElement() = default;

        // hierarchy management
        void addChild(std::shared_ptr<UIElement> child);

        // lifecycle methods
        virtual void update(float deltaTime);
        virtual void onEvent(const SDL_Event& event);

        virtual void generateGeometry(std::vector<UIVertex>& vertices, std::vector<uint16_t>& indices);

        // getters & setters
        const std::string& getName() const { return m_name; }
        glm::vec2 getAbsolutePosition() const;
        glm::vec2 getSize() const { return m_size; }

        void setPosition(glm::vec2 position) { m_position = position; }
        virtual void setSize(glm::vec2 size) { m_size = size; }

        glm::vec2 getPosition() const { return m_position; }

        glm::vec4 getColor() const { return m_color; }
        void setColor(glm::vec4 color) { m_color = color; }

        bool isVisible() const { return m_visible; }
        void setVisible(bool visible) { m_visible = visible; }

        bool drawsBackground() const { return m_drawsBackground; }
        void setDrawsBackground(bool draw) { m_drawsBackground = draw; }

        const std::vector<std::shared_ptr<UIElement>>& getChildren() const { return m_children; }

    protected:
        std::string m_name;
        glm::vec2 m_position;
        glm::vec2 m_size;

        glm::vec4 m_color{0.15f, 0.16f, 0.20f, 0.95f};
        bool m_visible{true};
        bool m_drawsBackground{false};

        std::weak_ptr<UIElement> m_parent;
        std::vector<std::shared_ptr<UIElement>> m_children;
    };

}