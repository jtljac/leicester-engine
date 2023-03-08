//
// Created by jacob on 27/02/23.
//

#pragma once


#include <vector>
#include "BaseProcessor.h"

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

class ImgProcessor : public BaseProcessor {
public:
    ImgProcessor();

    void processFile(const std::string& src, const std::string& dest) override;

    std::string getConversionMessage(const std::string& src, const std::string& dest) override;

    void writeTexture(const std::string& dest, uint32_t width, uint32_t height, TextureFormat format,
                 const unsigned char* pixels);
};
