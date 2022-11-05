//
// Created by jacob on 04/11/22.
//

#include "../ShaderProcessor.h"

#include <cstring>
#include <iostream>
#include <fstream>
#include <ios>
#include <filesystem>

LEIncluder::LEIncluder(const std::string& includePath) {
    if (includePath.empty() || !std::filesystem::is_directory(includePath)) {
        std::cout << "Include path " << includePath << " doesn't point to a directory, standard includes will not work" << std::endl;
        return;
    }

    this->includePath = includePath;
}

shaderc_include_result* LEIncluder::makeError(const char* message) {
    return new shaderc_include_result {"", 0, message, strlen(message), nullptr};
}

shaderc_include_result* LEIncluder::GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source,
                                               size_t include_depth) {
    std::filesystem::path filePath;

    if (type == shaderc_include_type_relative) {
        filePath = std::filesystem::path(requested_source).parent_path();
    } else if (!this->includePath.empty()){
        filePath = this->includePath;
    } else {
        return LEIncluder::makeError("Regular includes are disabled");
    }
    filePath /= requested_source;

    if (!exists(filePath)) {
        std::string message = "Failed to find requested ";
        message += (type == shaderc_include_type_relative ? "relative" : "regular");
        message += " file: ";
        message += requested_source;
        return LEIncluder::makeError(message.c_str());
    }

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

void LEIncluder::ReleaseInclude(shaderc_include_result* data) {
    delete (IncludeResultStorage*) data->user_data;
    delete data;
}


/* ==================================================================== */


ShaderProcessor::ShaderProcessor(const std::string& includePath) {
    options.SetIncluder(std::make_unique<LEIncluder>(includePath));
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
            sourceFile.c_str(), options
    );

    if (preprocessorResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cout << preprocessorResult.GetErrorMessage();
        return "";
    }

    return {preprocessorResult.begin(), preprocessorResult.end()};
}



std::vector<char> ShaderProcessor::compileShader(const std::string& shaderCode, shaderc_shader_kind kind, const std::string& src) {
    shaderc::CompilationResult compileResult = compiler.CompileGlslToSpv(
            shaderCode,
            kind,
            src.c_str()
    );

    if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cout << compileResult.GetErrorMessage();
        return {};
    }

    return {compileResult.begin(), compileResult.end()};
}

void ShaderProcessor::writeShader(const std::string& dest, const std::vector<char>& spv) {
    std::ofstream writer(dest);

    writer.write(spv.data(), spv.size());

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

    std::vector<char> spv = compileShader(processed, kind, src);

    writeShader(dest + ".spv", spv);
}

std::string ShaderProcessor::getConversionMessage(const std::string& src, const std::string& dest) {
    return "Compiling shader: " + src;
}