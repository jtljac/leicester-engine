//
// Created by jacob on 27/02/23.
//

#include "../Texture.h"

#define TEXTUREVERSION 1

int FormatUtils::calculatePixelSize(TextureFormat format) {
    int formatValue = (int) format;
    if (formatValue < 4) {
        return formatValue + 1;
    }
    return formatValue - 4 * 2;
}

Texture::Texture(int width, int height, TextureFormat format, const std::vector<char>& pixels) : width(width),
                                                                                                 height(height),
                                                                                                 format(format),
                                                                                                 pixels(pixels) {}

bool Texture::loadTextureFromFile(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        Logger::warn("Failed to load Texture at " + filePath);
        return false;
    }

    std::ifstream file(filePath, std::ios::binary);
    uint8_t version;
    file.read(reinterpret_cast<char*>(&version), 1);
    if (version != TEXTUREVERSION) {
        Logger::warn("Incorrect version of texture standard, this file cannot be read");
        file.close();
        return false;
    }

    file.read(reinterpret_cast<char*>(&this->width), 4);
    file.read(reinterpret_cast<char*>(&this->height), 4);

    file.read(reinterpret_cast<char*>(&this->format), 1);

    size_t textureSize = size();
    pixels.resize(textureSize);

    file.read(reinterpret_cast<char*>(&this->pixels), textureSize);

    file.close();

    return true;
}

Texture* Texture::createNewTextureFromFile(const std::string& filePath) {
    Texture* texture = new Texture;
    if (!texture->loadTextureFromFile(filePath)) {
        delete texture;
        return nullptr;
    }

    return texture;
}

int Texture::size() const {
    return this->width * this->height * FormatUtils::calculatePixelSize(this->format);
}
