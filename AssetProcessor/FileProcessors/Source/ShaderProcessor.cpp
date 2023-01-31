//
// Created by jacob on 04/11/22.
//

#include "../ShaderProcessor.h"

#include <cstring>
#include <iostream>
#include <fstream>
#include <ios>
#include <filesystem>

LEIncluder::LEIncluder(const std::vector<std::string>& includePaths) {
    for (const auto& includePath: includePaths) {
        if (!std::filesystem::is_directory(includePath))
            std::cout << "Include path (\"" << includePath << "\") doesn't point to a directory, it will be ignored" << std::endl;
        else {
            this->includePaths.push_back(includePath);
        }
    }

    if (this->includePaths.empty()) {
        std::cout << "No valid include paths have been provided, standard includes will be disabled" << std::endl;
    }
}

shaderc_include_result* LEIncluder::makeError(const char* message) {
    return new shaderc_include_result {"", 0, message, strlen(message), nullptr};
}

shaderc_include_result* LEIncluder::GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source,
                                               size_t include_depth) {
    std::filesystem::path filePath;

    if (type == shaderc_include_type_relative) {
        return handleRelativeInclude(std::filesystem::path(requesting_source).parent_path() / requested_source);
    } else if (!this->includePaths.empty()){
        return handleStandardInclude(std::filesystem::path(requested_source));
    } else {
        return LEIncluder::makeError("Regular includes are disabled");
    }
}

void LEIncluder::ReleaseInclude(shaderc_include_result* data) {
    delete (IncludeResultStorage*) data->user_data;
    delete data;
}

shaderc_include_result* LEIncluder::handleRelativeInclude(const std::filesystem::path& filePath) const {
    if (!exists(filePath)) {
        std::string message = "Failed to find requested relative file: ";
        message += filePath;
        return LEIncluder::makeError(message.c_str());
    }

    return getIncludeResult(filePath);
}

shaderc_include_result* LEIncluder::handleStandardInclude(const std::filesystem::path& filePath) const {
    for (const auto& includePath: includePaths) {
        std::filesystem::path path(includePath);
        path /= filePath;

        if (!exists(filePath)) continue;

        return getIncludeResult(path);
    }

    std::string message = "Failed to find any files in the include paths that match the path: ";
    message += filePath;
    return LEIncluder::makeError(message.c_str());
}

shaderc_include_result* LEIncluder::getIncludeResult(const std::filesystem::path& filePath) const {
    IncludeResultStorage* storage = new IncludeResultStorage {filePath, {}};
    {
        std::ifstream reader(filePath, std::ios::ate);
        size_t size = reader.tellg();
        reader.seekg(0);

        storage->contents.resize(size);
        reader.read(storage->contents.data(), size);
        reader.close();
    }

    return new shaderc_include_result {
            storage->filePath.c_str(),
            storage->filePath.size(),
            storage->contents.data(),
            storage->contents.size(),
            storage
    };
}


/* ==================================================================== */


ShaderProcessor::ShaderProcessor(std::vector<std::string> includePaths) {
    options.SetIncluder(std::make_unique<LEIncluder>(includePaths));
}

shaderc_shader_kind ShaderProcessor::kindFromExtension(const std::string& extension) {
    if (extension == "vert") return shaderc_vertex_shader;
    else if (extension == "frag") return shaderc_fragment_shader;
    return shaderc_glsl_infer_from_source;
}

std::string ShaderProcessor::loadShaderCode(const std::string& src) {
    std::ifstream reader(src);
    std::stringstream buffer;
    buffer << reader.rdbuf();
    reader.close();
    return buffer.str();
}

std::string ShaderProcessor::preprocessShader(const std::string& shaderCode, shaderc_shader_kind kind, const std::string& sourceFile) {
    shaderc::PreprocessedSourceCompilationResult preprocessorResult = compiler.PreprocessGlsl(
            shaderCode,
            kind,
            sourceFile.c_str(),
            options
    );

    if (preprocessorResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cout << preprocessorResult.GetErrorMessage();
        return "";
    }

    return {preprocessorResult.begin(), preprocessorResult.end()};
}



std::vector<uint32_t> ShaderProcessor::compileShader(const std::string& shaderCode, shaderc_shader_kind kind, const std::string& src) {
    shaderc::CompilationResult compileResult = compiler.CompileGlslToSpv(
            shaderCode,
            kind,
            src.c_str(),
            options
    );

    if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cout << compileResult.GetErrorMessage();
        return {};
    }

    return {compileResult.begin(), compileResult.end()};
}

void ShaderProcessor::writeShader(const std::string& dest, const std::vector<uint32_t>& spv) {
    std::ofstream writer(dest, std::ios::binary);

    writer.write(reinterpret_cast<const char*>(spv.data()), spv.size() * sizeof(uint32_t));

    writer.close();
}


void ShaderProcessor::processFile(const std::string& src, const std::string& dest) {
    std::string glslText = loadShaderCode(src);

    shaderc_shader_kind kind;
    {
        size_t extensionIndex = dest.find_last_of('.');
        if (extensionIndex == std::string::npos) kind = this->kindFromExtension("");
        else kind = this->kindFromExtension(dest.substr(extensionIndex + 1));
    }

    std::string processed = preprocessShader(glslText, kind, src);

    std::vector<uint32_t> spv = compileShader(processed, kind, src);

    writeShader(dest + ".spv", spv);
}

std::string ShaderProcessor::getConversionMessage(const std::string& src, const std::string& dest) {
    return "Compiling shader: " + src;
}