#pragma once

#include <SDL3/SDL_gpu.h>
#include <cstddef>
#include <filesystem>
#include <shaderc/shaderc.h>
#include <vector>

using namespace std;
using namespace std::filesystem;

enum ShaderStage
{
    Vertex,
    Fragment
};

struct ShaderUniformData
{
    void* data;
    size_t size;
};

class Shader
{

    /************************************************************************************************/
    /** Constructors */

public:

    Shader(string shaderSourcePath, ShaderStage shaderStage,
           int uniformBufferCount = 0);

    ~Shader();

    /************************************************************************************************/
    /** Shader */

protected:

    // Location of the source shader file (.glsl) on disk.
    path sourceFilePath;

    ShaderStage stage;

    size_t uniformBufferCount;

public:

    // Get or load SDL shader.
    virtual SDL_GPUShader* Load(SDL_GPUDevice* forDevice,
                                bool forceRegenerate = false);

    /************************************************************************************************/
    /** Shader Compilation (shaderc, glsl => spirv) */

private:

#define GLSL_ENTRY_POINT "main"

    // The shader code at @SourceFilePath compiled into SPIRV code.
    char* compiledShaderCode;

    // Size of @CompiledShaderCode in bytes;
    size_t compiledShaderLength;

    // Post compilation SDL compatible shader
    SDL_GPUShader* shaderCache;

protected:

    SDL_GPUShader* CompileShader(path sourceFile, SDL_GPUDevice* onDevice);

    bool CompileSourceToShaderCode(const char* sourceText,
                                   size_t sourceTextLength,
                                   char** outShaderCode,
                                   size_t* outShaderCodeLength);

    SDL_GPUShaderStage GetSDLShaderStage() const;

    shaderc_shader_kind GetShaderKind() const;
};
