#pragma once

#include "ui/ui_element.hpp"
#include "ui/ui_vertex.hpp"
#include "resources/font_loader.hpp"
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

        FontLoader* getFontLoader() { return &m_fontLoader; }

        bool loadFont(const std::string& fontId, const std::string& path, float pixelHeight) {
            auto loader = std::make_shared<FontLoader>();
            if (loader->loadFont(path, pixelHeight)) {
                m_fonts[fontId] = loader;
                if (!m_defaultFont) m_defaultFont = loader;
                return true;
            }
            return false;
        }

        std::shared_ptr<FontLoader> getFont(const std::string& fontId) {
            auto it = m_fonts.find(fontId);
            return (it != m_fonts.end()) ? it->second : m_defaultFont;
        }

    private:
        std::shared_ptr<UIElement> m_rootElement;
        glm::vec2 m_screenSize{0.0f, 0.0f};

        bool m_isDirty{true};
        std::vector<UIVertex> m_vertices;
        std::vector<uint16_t> m_indices;

        FontLoader m_fontLoader;

        std::unordered_map<std::string, std::shared_ptr<FontLoader>> m_fonts;
        std::shared_ptr<FontLoader> m_defaultFont;
    };

}