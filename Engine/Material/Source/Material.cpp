//
// Created by jacob on 12/11/22.
//

#include <utility>

#include "Material/Material.h"

Material::Material(std::string vertPath, std::string fragPath, ShaderType shaderType,
                   const std::vector<Texture*>& textures) :
                   materialStages({{vertPath, ShaderStage::VERT}, {fragPath, ShaderStage::FRAG}}),
                   shaderType(shaderType),
                   textures(textures)
                   {}
