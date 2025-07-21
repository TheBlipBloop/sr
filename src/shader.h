#pragma once

#include <SDL3/SDL_gpu.h>
#include <cstddef>
#include <filesystem>
#include <shaderc/shaderc.h>

using namespace std;
using namespace std::filesystem;

class Shader
{

    /************************************************************************************************/
    /** Constructors */

public:

    Shader();

    ~Shader();

    /************************************************************************************************/
    /** Shader */

public:

    // Get or load SDL shader.
    virtual SDL_GPUShader* Load(SDL_GPUDevice* forDevice,
                                bool forceRegenerate = false);

    /************************************************************************************************/
    /** Compilation */

private:

#define GLSL_ENTRY_POINT "main"

    // Post compilation SDL compatible shader.
    SDL_GPUShader* shaderCache;

    // The GPU device @shaderCache was loaded to.
    SDL_GPUDevice* deviceCache;

protected:

    bool CompileShader(SDL_GPUDevice* onDevice, SDL_GPUShader** outShader);

    bool CompileSourceToShaderCode(const char* sourceText,
                                   size_t sourceTextLength,
                                   char** outShaderCode,
                                   size_t* outShaderCodeLength);

    virtual string GetShaderSourceCode() const = 0;

    virtual int GetUniformBufferCount() const = 0;

    virtual SDL_GPUShaderStage GetSDLShaderStage() const = 0;

    virtual shaderc_shader_kind GetShaderKind() const = 0;
};
