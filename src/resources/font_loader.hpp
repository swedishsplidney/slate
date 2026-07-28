#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "stb_truetype.h"
#include "ui/ui_vertex.hpp"

namespace slate {

    struct CharacterInfo {
        glm::vec2 uvMin;
        glm::vec2 uvMax;
        glm::vec2 size;
        glm::vec2 bearing;
        float advance;
    };

    class FontLoader {
    public:
        FontLoader() = default;
        ~FontLoader() = default;
        
        bool loadFont(const std::string& fontPath, float pixelHeight, int atlasWidth = 512, int atlasHeight = 512);

        void generateTextGeometry(
            const std::string& text,
            glm::vec2 position,
            glm::vec4 color,
            std::vector<UIVertex>& outVertices,
            std::vector<uint16_t>& outIndices
        );

        const std::vector<unsigned char>& getAtlasBitmap() const { return m_atlasBitmap; }
        int getAtlasWidth() const { return m_atlasWidth; }
        int getAtlasHeight() const { return m_atlasHeight; }

    private:
        int m_atlasWidth{512};
        int m_atlasHeight{512};
        std::vector<unsigned char> m_atlasBitmap;
        stbtt_bakedchar m_cdata[96];
        stbtt_fontinfo m_fontInfo;
    };

}