#include "ui/ui_manager.hpp"
#include <functional>

namespace slate {

    UIManager::UIManager() = default;

    void UIManager::init(float screenWidth, float screenHeight) {
        m_screenSize = glm::vec2(screenWidth, screenHeight);
        m_rootElement = std::make_shared<UIElement>("RootCanvas", glm::vec2(0.0f), m_screenSize);
    }

    void UIManager::setScreenSize(float width, float height) {
        if (m_screenSize.x == width && m_screenSize.y == height) return;

        m_screenSize = glm::vec2(width, height);

        if (m_rootElement) {
            m_rootElement->setSize(m_screenSize);
        }

        markDirty();
    }

    void UIManager::update(float deltaTime) {
        if (m_rootElement) {
            m_rootElement->update(deltaTime);
        }
    }

    void UIManager::onEvent(const SDL_Event& event) {
        if (m_rootElement) {
            m_rootElement->onEvent(event);
        }
    }

    void UIManager::rebuildGeometry() {
        if (!m_isDirty || !m_rootElement) return;

        m_vertices.clear();
        m_indices.clear();

        std::function<void(const std::shared_ptr<UIElement>&)> appendElementGeometry =
            [&](const std::shared_ptr<UIElement>& element) {
                if (!element || !element->isVisible()) return;

                if (element->drawsBackground()) {
                    glm::vec2 pos = element->getAbsolutePosition();
                    glm::vec2 size = element->getSize();
                    glm::vec4 color = element->getColor();

                    uint16_t baseIndex = static_cast<uint16_t>(m_vertices.size());

                    m_vertices.push_back({ {pos.x, pos.y}, color });
                    m_vertices.push_back({ {pos.x + size.x, pos.y}, color });
                    m_vertices.push_back({ {pos.x + size.x, pos.y + size.y}, color });
                    m_vertices.push_back({ {pos.x, pos.y + size.y}, color });

                    m_indices.push_back(baseIndex + 0);
                    m_indices.push_back(baseIndex + 1);
                    m_indices.push_back(baseIndex + 2);
                    m_indices.push_back(baseIndex + 2);
                    m_indices.push_back(baseIndex + 3);
                    m_indices.push_back(baseIndex + 0);
                }

                for (const auto& child : element->getChildren()) {
                    appendElementGeometry(child);
                }
        };

        appendElementGeometry(m_rootElement);
        m_isDirty = false;
    }

}