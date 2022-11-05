//
// Created by jacob on 04/11/22.
//

#pragma once

#include <string>
#include <vector>
#include "../DataTypes/Vertex.h"

#include "BaseProcessor.h"

class ObjProcessor : public BaseProcessor {

    static void writeMesh(const std::string& dest, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

public:
    /**
     *
     * This function was written partially using the following guide:
     * https://marcelbraghetto.github.io/a-simple-triangle/2019/04/14/part-09/  MIT-License: https://github.com/MarcelBraghetto/a-simple-triangle/blob/master/LICENSE
     * @param src
     * @param dest
     */
    void processFile(const std::string &src, const std::string &dest) override;
    std::string getConversionMessage(const std::string& src, const std::string& dest) override;
};