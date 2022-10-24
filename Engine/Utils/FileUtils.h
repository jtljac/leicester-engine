//
// Created by jacob on 24/10/22.
//

#pragma once

#include <string>

struct FileUtils {
    /**
     * Get the path to the executable
     * @return The path to the executable
     */
    static std::string getGamePath();

    /**
     * Get the path containing all the assets
     * @return the path containing all the game assets
     */
    static std::string getAssetsPath();

    /**
     * Get the working directory
     * @return The working directory
     */
    static std::string getWorkingDirectory();
};
