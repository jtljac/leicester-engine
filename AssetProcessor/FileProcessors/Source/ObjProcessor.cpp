#include <iostream>
#include <vector>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_EXT_INCLUDED
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include "../../DataTypes/Vertex.h"

#include "../ObjProcessor.h"

void ObjProcessor::processFile(const std::string& src, const std::string& dest) {
    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.vertex_color = true;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(src, readerConfig)) {
        std::cout << "Failed to convert obj file: " << src << std::endl;
        if (!reader.Error().empty()) {
            std::cout << reader.Error() << std::endl;
        }
        return;
    }

    if (!reader.Warning().empty()) {
        std::cout << reader.Warning() << std::endl;
    }

    const tinyobj::attrib_t& attrib = reader.GetAttrib();

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<glm::vec3, uint32_t> uniqueVertices;

    for (const tinyobj::shape_t& shape : reader.GetShapes()) {
        for (const tinyobj::index_t& index : shape.mesh.indices) {
            glm::vec3 position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
            };
            if (uniqueVertices.count(position) == 0) {
                uniqueVertices[position] = static_cast<uint32_t>(vertices.size());
                vertices.push_back({
                                           position,
                                           {
                                                   attrib.normals[3*index.normal_index + 0],
                                                   attrib.normals[3*index.normal_index + 1],
                                                   attrib.normals[3*index.normal_index + 2]
                                           },
                                           {
                                                   attrib.colors[3*index.vertex_index + 0],
                                                   attrib.colors[3*index.vertex_index + 1],
                                                   attrib.colors[3*index.vertex_index + 2]
                                           }
                                   });
            }

            indices.push_back(uniqueVertices[position]);
        }
    }
    ObjProcessor::writeMesh(replaceExtension(dest, "lmesh"), vertices, indices);
}

void ObjProcessor::writeMesh(const std::string& dest, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    std::ofstream destFile(dest);
    uint32_t vertexCount = vertices.size();
    uint32_t indexCount = indices.size();

    // Write Header
    {
        uint8_t version = 1;
        // Version
        destFile.write(reinterpret_cast<char*>(&version), 1);
        // Vertex Count
        destFile.write(reinterpret_cast<char*>(&vertexCount), 4);
        // Index Count
        destFile.write(reinterpret_cast<char*>(&indexCount), 4);
    }

    // Write Body
    {
        destFile.write(reinterpret_cast<char*>(vertices.data()), vertexCount * sizeof(Vertex));
        destFile.write(reinterpret_cast<char*>(indices.data()), indexCount * sizeof(uint32_t));
    }
    destFile.close();
}

std::string ObjProcessor::getConversionMessage(const std::string& src, const std::string& dest) {
    return "Converting " + src + " to lmesh " + dest;
}
