#include "font_loader.hpp"
#include <fstream>
#include <iostream>

namespace slate {

    bool FontLoader::loadFont(const std::string& fontPath, float pixelHeight, int atlasWidth, int atlasHeight) {
        m_atlasWidth = atlasWidth;
        m_atlasHeight = atlasHeight;

        std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "[fontloader] failed to open font: " << fontPath << std::endl;
            return false;
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<unsigned char> fontBuffer(fileSize);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(fontBuffer.data()), fileSize);
        file.close();

        m_atlasBitmap.resize(m_atlasWidth * m_atlasHeight);

        int result = stbtt_BakeFontBitmap(
            fontBuffer.data(), 0,
            pixelHeight,
            m_atlasBitmap.data(), m_atlasWidth, m_atlasHeight,
            32, 96,
            m_cdata
        );

        if (result <= 0) {
            std::cerr << "[fontloader] font atlas too small for specified pixel height!" << std::endl;
            return false;
        }

        std::cout << "[fontloader] successfully loaded font: " << fontPath << std::endl;
        return true;
    }

    void FontLoader::generateTextGeometry(
        const std::string& text, 
        glm::vec2 position, 
        glm::vec4 color, 
        std::vector<UIVertex>& outVertices, 
        std::vector<uint16_t>& outIndices
    ) {
        float x = position.x;
        float y = position.y;

        for (char c : text) {
            if (c < 32 || c >= 128) continue;

            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(
                m_cdata, 
                m_atlasWidth, m_atlasHeight, 
                c - 32, 
                &x, &y, 
                &q, 
                1
            );

            uint16_t baseIndex = static_cast<uint16_t>(outVertices.size());

            outVertices.push_back({ {q.x0, q.y0}, color });
            outVertices.push_back({ {q.x1, q.y0}, color });
            outVertices.push_back({ {q.x1, q.y1}, color });
            outVertices.push_back({ {q.x0, q.y1}, color });

            outIndices.push_back(baseIndex + 0);
            outIndices.push_back(baseIndex + 1);
            outIndices.push_back(baseIndex + 2);
            outIndices.push_back(baseIndex + 2);
            outIndices.push_back(baseIndex + 3);
            outIndices.push_back(baseIndex + 0);
        }
    }

}