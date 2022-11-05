//
// Created by jacob on 04/11/22.
//

#pragma once

#include <shaderc/shaderc.hpp>
#include "BaseProcessor.h"

class LEIncluder : public shaderc::CompileOptions::IncluderInterface {
    std::string includePath;

    static shaderc_include_result* makeError(const char* message);

    struct IncludeResultStorage {
        std::string filePath;
        std::vector<char> contents;
    };
public:
    LEIncluder(const std::string& includePath);
    shaderc_include_result*
    GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source,
               size_t include_depth) override;

    void ReleaseInclude(shaderc_include_result* data) override;
    ~LEIncluder() override = default;
};

class ShaderProcessor : public BaseProcessor {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    shaderc_shader_kind kindFromExtension(const std::string& extension);
    static std::string loadShaderCode(const std::string& src);
    std::string preprocessShader(const std::string& shaderCode, shaderc_shader_kind kind, const std::string& sourceFile);
    std::vector<char> compileShader(const std::string& shaderCode, shaderc_shader_kind kind, const std::string& src);
    static void writeShader(const std::string& dest, const std::vector<char>& spv);

public:
    ShaderProcessor() = default;
    ShaderProcessor(const std::string& includePath);
    void processFile(const std::string& src, const std::string& dest) override;
    std::string getConversionMessage(const std::string& src, const std::string& dest) override;
};