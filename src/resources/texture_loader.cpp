#include "texture_loader.hpp"

#include <iostream>
#include <vector>
#include <future>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

LoadedImage TextureLoader::loadImage(const std::string& filepath) {
    LoadedImage img;

    int desiredChannels = 4;
    img.pixels = stbi_load(filepath.c_str(), &img.width, &img.height, &img.channels, desiredChannels);
    img.channels = desiredChannels;

    if (!img.pixels) {
        std::cerr << "[textureloader] failed to load image: " << filepath << " | reason: " << stbi_failure_reason() << std::endl;
        img.success = false;
    } else {
        img.success = true;
    }

    return img;
}

void TextureLoader::freeImage(LoadedImage& image) {
    if (image.pixels) {
        stbi_image_free(image.pixels);
        image.pixels = nullptr;
        image.success = false;
    }
}

std::vector<LoadedImage> TextureLoader::loadImagesParallel(const std::vector<std::string>& filepaths) {
    std::vector<std::future<LoadedImage>> futures;
    futures.reserve(filepaths.size());

    for (const auto& path : filepaths) {
        futures.push_back(std::async(std::launch::async, [path]() {
            return loadImage(path);
        }));
    }

    std::vector<LoadedImage> results;
    results.reserve(filepaths.size());

    for (auto& fut : futures) {
        results.push_back(fut.get());
    }

    return results;
}