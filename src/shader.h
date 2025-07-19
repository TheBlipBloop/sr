#pragma once

#include <SDL3/SDL_gpu.h>
#include <filesystem>
#include <shaderc/shaderc.h>

using namespace std;
using namespace std::filesystem;

enum ShaderStage
{
    Vertex,
    Fragment
};

class Shader
{
    /************************************************************************************************/
    /** Shader */

protected:

    // Location of the source shader file (.glsl) on disk.
    path SourceFilePath;

    ShaderStage Stage;

public:

    // Get or load SDL shader.
    virtual SDL_GPUShader* Load(SDL_GPUDevice* forDevice);

    /************************************************************************************************/
    /** Constructors */

public:

    Shader(string shaderSourcePath, ShaderStage shaderStage);

    ~Shader();

    /************************************************************************************************/
    /** Shader Compilation (shaderc, glsl => spirv) */

private:

#define GLSL_ENTRY_POINT "main"

    // The shader code at @SourceFilePath compiled into SPIRV code.
    char* CompiledShaderCode;

    // Size of @CompiledShaderCode in bytes;
    size_t CompiledShaderLength;

    // Post compilation SDL compatible shader
    SDL_GPUShader* ShaderCache;

protected:

    SDL_GPUShader* CompileShader(path sourceFile, SDL_GPUDevice* onDevice);

    bool CompileSourceToShaderCode(const char* sourceText,
                                   size_t sourceTextLength,
                                   char** outShaderCode,
                                   size_t* outShaderCodeLength);

    SDL_GPUShaderStage GetSDLShaderStage() const;

    shaderc_shader_kind GetShaderKind() const;
};
