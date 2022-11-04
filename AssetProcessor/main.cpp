#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_EXT_INCLUDED
#include <glm/gtx/hash.hpp>
#include "DataTypes/Vertex.h"

/**
 *
 * This function was written partially using the following guide:
 * https://marcelbraghetto.github.io/a-simple-triangle/2019/04/14/part-09/  MIT-License: https://github.com/MarcelBraghetto/a-simple-triangle/blob/master/LICENSE
 * @param src
 * @param dest
 */
void processObj(const std::string& src, const std::string& dest) {
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

void processFiles(const std::string& src, const std::string& dest) {
    if (std::filesystem::exists(dest)) {
        std::filesystem::remove_all(dest);
        std::filesystem::create_directory(dest);
    }

    for (const auto& file : std::filesystem::recursive_directory_iterator(src)) {
        if (file.is_directory()) continue;
        std::string extension = file.path().extension();
        std::filesystem::path relative = std::filesystem::relative(file.path(), src);
        std::filesystem::path destPath = std::filesystem::path(dest) / relative;

        if (!exists(destPath.parent_path())) std::filesystem::create_directories(destPath.parent_path());
        if (extension == ".obj") {
            std::cout << "Converting " << file.path() << " to lmesh" << std::endl;
            processObj(file.path(), destPath.replace_extension("lmesh"));
        } else {
            std::cout << "Copying " << file.path() << std::endl;
            std::filesystem::copy_file(file.path(), destPath, std::filesystem::copy_options::overwrite_existing);
        }
    }
}


int main(int argc, char** argv) {
    // Arg1 source directory
    // Arg2 destination directory
    if (argc < 3) {
        std::cout << "You must provide the source directory and the destination directory as arguments" << std::endl;
        return 1;
    }
    std::string src = argv[1];
    std::string dest = argv[2];

    if (!std::filesystem::exists(src)) {
        std::cout << "Failed to find source directory" << std::endl;
        return 1;
    }

    std::cout << "==========Starting Asset Processor==========" << std::endl;
    processFiles(src, dest);
    std::cout << "==========Completed Asset Processor==========" << std::endl;
    return 0;
}