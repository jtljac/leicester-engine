#include <iostream>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <cstring>
#include "FileProcessors/ObjProcessor.h"
#include "FileProcessors/CopyProcessor.h"

std::unordered_map<std::string, BaseProcessor*> processorMap;
CopyProcessor fallbackProcessor;


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
        BaseProcessor* processor;
        if (processorMap.count(extension) != 0) {
             processor = processorMap.at(extension);
        } else {
            processor = &fallbackProcessor;
        }

        std::cout << processor->getConversionMessage(file.path(), destPath) << std::endl;
        processor->processFile(file.path(), destPath);
    }
}


int main(int argc, char** argv) {
    // Arg1 source directory
    // Arg2 destination directory
    if (argc < 3) {
        std::cout << "You must provide the source directory and the destination directory as arguments" << std::endl;
        return 1;
    } else if (argc < 4 || strlen(argv[3]) == 0) {
//        std::cout << "By not providing a shader include path, regular shader includes will not work" << std::endl;
    }
    std::string src = argv[1];
    std::string dest = argv[2];
    std::string shaderIncludePath = (argc > 3 ? argv[3] : "");

    if (!std::filesystem::exists(src)) {
        std::cout << "Failed to find source directory" << std::endl;
        return 1;
    }
    // Register Processors
    {
//        auto* shaderProcessor = new ShaderProcessor(shaderIncludePath);
//        processorMap.emplace(".frag", shaderProcessor);
//        processorMap.emplace(".vert", shaderProcessor);

        processorMap.emplace(".obj", new ObjProcessor());
    }

    std::cout << "==========Starting Asset Processor==========" << std::endl;
    processFiles(src, dest);
    std::cout << "==========Completed Asset Processor==========" << std::endl;
    return 0;
}