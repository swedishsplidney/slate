#pragma once

#include "ui/ui_element.hpp"
#include "ui/ui_vertex.hpp"
#include <memory>
#include <vector>
#include <SDL3/SDL_events.h>

namespace slate {

    class UIManager {
    public:
        UIManager();
        ~UIManager() = default;

        void init(float screenWidth, float screenHeight);
        void update(float deltaTime);
        void onEvent(const SDL_Event& event);

        // screen resizing
        void setScreenSize(float width, float height);

        // tree
        void setRootElement(std::shared_ptr<UIElement> root) { m_rootElement = root; }
        std::shared_ptr<UIElement> getRootElement() const { return m_rootElement; }

        // batching
        bool isDirty() const { return m_isDirty; }
        void markDirty() { m_isDirty = true; }

        const std::vector<UIVertex>& getVertexBuffer() const { return m_vertices; }
        const std::vector<uint16_t>& getIndexBuffer() const { return m_indices; }

        void rebuildGeometry();

    private:
        std::shared_ptr<UIElement> m_rootElement;
        glm::vec2 m_screenSize{0.0f, 0.0f};

        bool m_isDirty{true};
        std::vector<UIVertex> m_vertices;
        std::vector<uint16_t> m_indices;
    };

}