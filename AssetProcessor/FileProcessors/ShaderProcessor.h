//
// Created by jacob on 04/11/22.
//

#pragma once

#include <shaderc/shaderc.hpp>
#include <filesystem>
#include "BaseProcessor.h"

class LEIncluder : public shaderc::CompileOptions::IncluderInterface {
    std::vector<std::string> includePaths;

    /**
     * Make an include result that contains an error message
     * @param message The error message
     * @return a pointer to a shaderc include result containing an error
     */
    static shaderc_include_result* makeError(const char* message);

    /**
     * Handle a relative include
     * @param filePath The absolute path to the file
     * @return a pointer to a shaderc include result containing an error or the include data
     */
    [[nodiscard]] shaderc_include_result* handleRelativeInclude(const std::filesystem::path& filePath) const;

    /**
     * Handle a standard include
     * @param filePath The relative filepath to the file
     * @return a pointer to a shaderc include result containing an error or the include data
     */
    [[nodiscard]] shaderc_include_result* handleStandardInclude(const std::filesystem::path& filePath) const;

    /**
     * Build an include result using the file.
     * Assumes the filepath is valid
     * @param filePath The path to the file
     * @return a pointer to a shaderc include result containing the include data
     */
    [[nodiscard]] shaderc_include_result* getIncludeResult(const std::filesystem::path& filePath) const;

    struct IncludeResultStorage {
        std::string filePath;
        std::vector<char> contents;
    };
public:
    explicit LEIncluder(const std::vector<std::string>& includePaths);
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
    std::vector<uint32_t> compileShader(const std::string& shaderCode, shaderc_shader_kind kind, const std::string& src);
    static void writeShader(const std::string& dest, const std::vector<uint32_t>& spv);

public:
    ShaderProcessor() = default;
    explicit ShaderProcessor(std::vector<std::string> includePaths);
    void processFile(const std::string& src, const std::string& dest) override;
    std::string getConversionMessage(const std::string& src, const std::string& dest) override;
};