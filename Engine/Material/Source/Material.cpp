//
// Created by jacob on 12/11/22.
//

#include <utility>

#include "Material/Material.h"

Material::Material(std::string  vertPath, std::string  fragPath) :
        spirvVert(std::move(vertPath)),
        spirvFrag(std::move(fragPath)),
        wireframe(false) {}

Material::Material(std::string vertPath, std::string fragPath, bool wireframe):
        spirvVert(std::move(vertPath)),
        spirvFrag(std::move(fragPath)),
        wireframe(wireframe) {}
