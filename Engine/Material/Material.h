//
// Created by jacob on 12/11/22.
//

#pragma once


#include <cstdint>
#include <string>

#include <Texture/Texture.h>

enum class ShaderStage : uint8_t {
    FRAG,
    VERT
};

enum class ShaderType : uint8_t {
    OPAQUE,
    WIREFRAME,
    TRANSPARENT
};

struct MaterialStage {
    std::string shaderPathSpirv;
    ShaderStage shaderStage;
};

struct Material {
    uint64_t materialId = 0;

    std::vector<MaterialStage> materialStages;
    ShaderType shaderType = ShaderType::OPAQUE;

    std::vector<Texture*> textures;

    Material(std::string vertPath, std::string fragPath, ShaderType shaderType, const std::vector<Texture*>& textures = {});
};
