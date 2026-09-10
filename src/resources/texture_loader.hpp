#pragma once

#include <string>

struct LoadedImage {
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;
    bool success = false;
};

class TextureLoader {
public:
    static LoadedImage loadImage(const std::string& filepath);

    static void freeImage(LoadedImage& image);
};