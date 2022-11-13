//
// Created by jacob on 13/11/22.
//

#pragma once


#include "Mesh.h"
#include "Material/Material.h"

#include <glm/mat4x4.hpp>

struct StaticMesh {
    Mesh* mesh;
    Material* material;
    glm::mat4 transform;
};
