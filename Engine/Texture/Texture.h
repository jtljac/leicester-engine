//
// Created by jacob on 27/02/23.
//
#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include "Utils/Logger.h"

enum class TextureFormat : uint8_t {
    R8 = 0,
    R8G8 = 1,
    R8G8B8 = 2,
    R8G8B8A8 = 3,
    R16 = 4,
    R16G16 = 5,
    R16G16B16 = 6,
    R16G16B16A16 = 7
};

namespace FormatUtils {
    /**
     * Calculate the size per pixel for the given format
     * @param format The format
     * @return The size of a pixel in the given format
     */
    int calculatePixelSize(TextureFormat format);
}

struct Texture {
    unsigned int width;
    unsigned int height;
    TextureFormat format;

    std::vector<char> pixels;

    size_t textureId = 0;

    Texture() = default;

    Texture(int width, int height, TextureFormat format, const std::vector<char>& pixels);



    bool loadTextureFromFile(const std::string& filePath);

    static Texture* createNewTextureFromFile(const std::string& filePath);

    [[nodiscard]] int size() const;
};
