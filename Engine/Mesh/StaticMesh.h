//
// Created by jacob on 13/11/22.
//

#pragma once


#include "Mesh.h"
#include "Material/Material.h"

struct StaticMesh {
    Mesh* mesh;
    Material* material;

    StaticMesh(Mesh* mesh, Material* material);
};
