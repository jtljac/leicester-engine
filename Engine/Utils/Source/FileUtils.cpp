//
// Created by jacob on 24/10/22.
//

#include "Utils/FileUtils.h"

std::string FileUtils::getAssetsPath() {
    return FileUtils::getGamePath() + "/Assets";
}
