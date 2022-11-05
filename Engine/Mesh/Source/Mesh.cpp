//
// Created by jacob on 13/10/22.
//

#include "Mesh/Mesh.h"

#include <Utils/Logger.h>

#include <fstream>
#include <filesystem>
#define MESHVERSION 1

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
    return true;
}
