#include <iostream>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <cstring>
#include "FileProcessors/ObjProcessor.h"
#include "FileProcessors/ShaderProcessor.h"
#include "FileProcessors/CopyProcessor.h"

std::unordered_map<std::string, BaseProcessor*> processorMap;
CopyProcessor fallbackProcessor;


void processFiles(const std::vector<std::string>& srcs, const std::string& dest) {
    if (std::filesystem::exists(dest)) {
        std::filesystem::remove_all(dest);
        std::filesystem::create_directory(dest);
    }

    for (const auto& src: srcs) {
        for (const auto& file: std::filesystem::recursive_directory_iterator(src)) {
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
}


int main(int argc, char** argv) {
    // Arg1         engine assets directory
    // Arg2         project assets directory
    // Arg3         destination directory
    // Arg4 -> Argn shader standard include directories
    if (argc < 4) {
        std::cout << "You must provide the source directory, project source directory, and the destination directory as arguments" << std::endl;
        return 1;
    } else if (argc < 5) {
        std::cout << "By not providing any shader include paths, regular shader includes will not work" << std::endl;
    }

    std::vector<std::string> srcs;
    if (std::filesystem::is_directory(argv[1])) srcs.emplace_back(argv[1]);
    if (std::filesystem::is_directory(argv[2])) srcs.emplace_back(argv[2]);

    if (!std::filesystem::is_directory(srcs[0]) && !std::filesystem::is_directory(srcs[1])) {
        std::cout << "Neither an engine asset directory or a project asset directory point to valid folders, the asset processor"
                     "cannot run" << std::endl;
        return 2;
    }

    std::string dest = argv[3];
    std::vector<std::string> shaderIncludePaths{argv + 4, argv + argc};

    // Register Processors
    {
        auto* shaderProcessor = new ShaderProcessor(shaderIncludePaths);
        processorMap.emplace(".frag", shaderProcessor);
        processorMap.emplace(".vert", shaderProcessor);

        processorMap.emplace(".obj", new ObjProcessor());
    }

    std::cout << "==========Starting Asset Processor==========" << std::endl;
    processFiles(srcs, dest);
    std::cout << "==========Completed Asset Processor==========" << std::endl;
    return 0;
}