//
// Created by jacob on 12/11/22.
//

#pragma once


#include <cstdint>
#include <string>

struct Material {
    uint64_t materialId = 0;

    std::string spirvVert;
    std::string spirvFrag;

    Material(std::string vertPath, std::string fragPath);
};
