//
// Created by jacob on 04/11/22.
//

#pragma once
#include <string>

class BaseProcessor {
public:
    virtual void processFile(const std::string& src, const std::string& dest) = 0;

    virtual std::string getConversionMessage(const std::string& src, const std::string& dest) = 0;

    static std::string replaceExtension(const std::string& path, const std::string& newExtension) {
        std::string parent;
        std::string fileName;
        {
            size_t slashIndex = path.find_last_of("\\/");
            if (slashIndex == std::string::npos) {
                parent = "";
                fileName = path;
            } else {
                parent = path.substr(0, slashIndex);
                fileName = path.substr(slashIndex + 1);
            }
        }
        std::string nameWithoutExtension;
        {
            size_t extensionIndex = fileName.find_last_of('.');
            if (extensionIndex == std::string::npos) nameWithoutExtension = fileName;
            else nameWithoutExtension = fileName.substr(0, extensionIndex);
        }
        return parent + "/" + nameWithoutExtension + "." + newExtension;
    }
};