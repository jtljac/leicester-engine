//
// Created by jacob on 13/10/22.
//

#pragma once
#include <vector>
#include <string>
#include <cstdint>

#include "Vertex.h"

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    size_t verticesId;
    size_t indicesId;

    bool loadMeshFromFile(const std::string& filePath);
};