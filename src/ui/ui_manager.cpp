#include "ui/ui_manager.hpp"
#include <functional>
#include <iostream>

namespace slate {

    UIManager::UIManager() = default;

    void UIManager::init(float screenWidth, float screenHeight) {
        m_screenSize = glm::vec2(screenWidth, screenHeight);

        setRootElement(
            std::make_shared<UIElement>(
                "RootCanvas",
                glm::vec2(0.0f),
                m_screenSize
            )
        );
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

    void UIManager::setRootElement(std::shared_ptr<UIElement> root) {
        m_rootElement = std::move(root);

        if (m_rootElement) {
            m_rootElement->setDirtyCallback([this]() {
                markDirty();
            });
        }

        markDirty();
    }

    void UIManager::rebuildGeometry() {
        if (!m_isDirty || !m_rootElement) return;
        m_vertices.clear(); m_indices.clear();

        std::vector<std::shared_ptr<UIElement>> overlays;
        std::function<void(const std::shared_ptr<UIElement>&)> draw =
            [&](const std::shared_ptr<UIElement>& e) {
                if (!e || !e->isVisible()) return;
                if (e->isOverlay()) { overlays.push_back(e); return; }
                e->generateGeometry(m_vertices, m_indices);
                for (const auto& c : e->getChildren()) draw(c);
        };

        draw(m_rootElement);
        for (size_t i = 0; i < overlays.size(); ++i) {
            overlays[i]->generateGeometry(m_vertices, m_indices);
            for (const auto& c : overlays[i]->getChildren()) draw(c);
        }
        m_isDirty = false;
    }

}