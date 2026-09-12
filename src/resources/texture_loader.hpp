#pragma once

#include <string>
#include <vector>
#include <future>

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

    // concurrency is good frfr
    static std::vector<LoadedImage> loadImagesParallel(const std::vector<std::string>& filepaths);
};