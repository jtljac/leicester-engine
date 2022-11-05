//
// Created by jacob on 05/11/22.
//

#include "../CopyProcessor.h"
#include <filesystem>


void CopyProcessor::processFile(const std::string& src, const std::string& dest) {
    std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing);
}

std::string CopyProcessor::getConversionMessage(const std::string& src, const std::string& dest) {
    return "Copying " + src;
}
