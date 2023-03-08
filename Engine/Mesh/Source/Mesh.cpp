//
// Created by jacob on 13/10/22.
//

#include "Mesh/Mesh.h"

#include <Utils/Logger.h>

#include <fstream>
#include <filesystem>
#include <utility>
#define MESHVERSION 2

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices) : vertices(std::move(vertices)),
                                                                          indices(std::move(indices)),
                                                                          boundingBox(calculateBoundingBox()) {}

bool Mesh::loadMeshFromFile(const std::string& filePath) {
    if (!std::filesystem::exists(filePath)) {
        Logger::warn("Failed to load mesh at " + filePath);
        return false;
    }
    std::ifstream file(filePath, std::ios::binary);
    uint8_t version;
    uint32_t vertexCount, indexCount;
    file.read(reinterpret_cast<char*>(&version), 1);
    if (version != MESHVERSION) {
        Logger::warn("Incorrect version of mesh standard, this file cannot be read");
        file.close();
        return false;
    }

    file.read(reinterpret_cast<char*>(&vertexCount), 4);
    file.read(reinterpret_cast<char*>(&indexCount), 4);

    this->vertices.resize(vertexCount);
    this->indices.resize(indexCount);

    file.read(reinterpret_cast<char*>(vertices.data()), vertexCount * sizeof(Vertex));
    file.read(reinterpret_cast<char*>(indices.data()), indexCount * sizeof(uint32_t));

    file.close();

    this->boundingBox = calculateBoundingBox();
    return true;
}


Mesh* Mesh::createNewMeshFromFile(const std::string& filePath) {
    Mesh* mesh = new Mesh;
    if (!mesh->loadMeshFromFile(filePath)) {
        delete mesh;
        return nullptr;
    }
    return mesh;
}

BoundingBox Mesh::calculateBoundingBox() const {
    float maxX = 0, maxY = 0, maxZ = 0;
    float minX = 0, minY = 0, minZ = 0;
    for (const auto& vertex: vertices) {
        if (vertex.position.x > maxX) maxX = vertex.position.x;
        else if (vertex.position.x < minX) minX = vertex.position.x;
        if (vertex.position.y > maxY) maxY = vertex.position.y;
        else if (vertex.position.y < minY) minY = vertex.position.y;
        if (vertex.position.z > maxZ) maxZ = vertex.position.z;
        else if (vertex.position.z > minZ) minZ = vertex.position.z;

    }
    return {
            {minX, minY, minZ},
            {maxX, maxY, maxZ}
    };
}
