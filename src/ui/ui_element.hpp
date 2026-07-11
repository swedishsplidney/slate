#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <string>
#include <memory>
#include <SDL3/SDL_events.h>

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

        // getters
        const std::string& getName() const { return m_name; }
        glm::vec2 getAbsolutePosition() const;

    protected:
        std::string m_name;
        glm::vec2 m_position;
        glm::vec2 m_size;

        std::weak_ptr<UIElement> m_parent;
        std::vector<std::shared_ptr<UIElement>> m_children;
    };

}