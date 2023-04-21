#include <iostream>
#include <vector>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_EXT_INCLUDED
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>
#include <numeric>

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
                        {0, 0, 0}, // Placeholder
                        {
                                attrib.colors[3*index.vertex_index + 0],
                                attrib.colors[3*index.vertex_index + 1],
                                attrib.colors[3*index.vertex_index + 2]
                        },
                        {
                                attrib.texcoords[2 * index.texcoord_index + 0],
                               -attrib.texcoords[2 * index.texcoord_index + 1],

                        }
                });
            }
            indices.push_back(uniqueVertices[position]);
        }
    }

    /*
        calculate tangents
        Source: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
        License: CC BY-NC 4.0
     */
    std::vector<std::vector<glm::vec3>> tangents(vertices.size());
    for (int i = 0; i < indices.size(); i += 3) {
        Vertex a = vertices.at(indices[i+0]);
        Vertex b = vertices.at(indices[i+1]);
        Vertex c = vertices.at(indices[i+2]);

        glm::vec3 edge1 = b.pos - a.pos;
        glm::vec3 edge2 = c.pos - a.pos;
        glm::vec2 deltaUV1 = b.uv - a.uv;
        glm::vec2 deltaUV2 = c.uv - a.uv;

        float f = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        glm::vec3 tangent = {
                f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
        };

        tangents.at(indices[i+0]).push_back(tangent);
        tangents.at(indices[i+1]).push_back(tangent);
        tangents.at(indices[i+2]).push_back(tangent);
    }

    for (int i = 0; i < tangents.size(); ++i) {
        std::vector<glm::vec3>& tangentList = tangents.at(i);
        vertices.at(i).tangent = std::reduce(tangentList.begin(), tangentList.end()) / (float) tangentList.size();
    }

    ObjProcessor::writeMesh(replaceExtension(dest, "lmesh"), vertices, indices);
}

void ObjProcessor::writeMesh(const std::string& dest, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    std::ofstream destFile(dest, std::ios::binary);
    uint32_t vertexCount = vertices.size();
    uint32_t indexCount = indices.size();

    // Write Header
    {
        uint8_t version = 2;
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
    return "Converting Obj " + src + " to lmesh " + replaceExtension(dest, "lmesh");
}
