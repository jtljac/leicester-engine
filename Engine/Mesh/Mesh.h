//
// Created by jacob on 13/10/22.
//

#pragma once
#include <vector>
#include <string>
#include <cstdint>

#include "Vertex.h"
#include <Engine/BoundingBox.h>

struct Mesh {
protected:
    /**
     * Calculate the bounding box for this mesh
     */
    BoundingBox calculateBoundingBox() const;

public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    BoundingBox boundingBox;
    size_t verticesId = 0;
    size_t indicesId = 0;

    Mesh() = default;
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

    bool loadMeshFromFile(const std::string& filePath);

    static Mesh* createNewMeshFromFile(const std::string& filePath);
};