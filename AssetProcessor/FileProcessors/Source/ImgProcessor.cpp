//
// Created by jacob on 27/02/23.
//

#include "../ImgProcessor.h"


#include <vector>
#include <fstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

ImgProcessor::ImgProcessor() {
    stbi_convert_iphone_png_to_rgb(1);
}

void ImgProcessor::processFile(const std::string& src, const std::string& dest) {
    int width, height, channels;
    unsigned char* data = stbi_load(src.data(), &width, &height, &channels, 4);

    if (data == nullptr) {
        std::cout << "Failed to convert obj file: " << src << std::endl;
        std::cout << stbi_failure_reason() << std::endl;
        return;
    }

    // channel-1 should map to the 8 bits per channel section
    writeTexture(replaceExtension(dest, "ltex"), width, height, TextureFormat::R8G8B8A8, data);

    stbi_image_free(data);
}

void ImgProcessor::writeTexture(const std::string& dest, uint32_t width, uint32_t height, TextureFormat format, const unsigned char* pixels) {
    std::ofstream destFile(dest, std::ios::binary);

    // Write Header
    {
        uint8_t version = 1;
        // Version
        destFile.write(reinterpret_cast<char*>(&version), 1);
        // Width
        destFile.write(reinterpret_cast<char*>(&width), 4);
        // Height
        destFile.write(reinterpret_cast<char*>(&height), 4);

        uint8_t formatInt = static_cast<uint32_t>(format);

        // Format
        destFile.write(reinterpret_cast<char*>(&formatInt), 1);
    }

    // Write Body
    {
        // format+1, for channels of size 8, should map 1 to 1 to the size of the pixel in bytes
        destFile.write(reinterpret_cast<const char*>(pixels), width * height * (int) format + 1);
    }
    destFile.close();
}

std::string ImgProcessor::getConversionMessage(const std::string& src, const std::string& dest) {
    return "Converting Image " + src + " to ltex " + replaceExtension(dest, "ltex");
}