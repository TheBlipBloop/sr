#pragma once

#include <cstddef>
#include <string>

#include <SDL3/SDL_gpu.h>
#include <shaderc/shaderc.h>

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
    virtual SDL_GPUShader* Load(SDL_GPUDevice* for_device,
                                bool force_regenerate = false);

    /************************************************************************************************/
    /** Compilation */

private:

    const std::string GLSL_ENTRY_POINT = "main";

    // Post compilation SDL compatible shader.
    SDL_GPUShader* m_shader_cache;

    // The GPU device @shaderCache was loaded to.
    SDL_GPUDevice* m_device_cache;

protected:

    bool CompileShader(SDL_GPUDevice* for_device, SDL_GPUShader** out_shader);

    bool CompileSourceToShaderCode(const char* source_text,
                                   size_t source_text_length,
                                   char** out_shader_code,
                                   size_t* out_shader_code_length);

    virtual std::string GetShaderSourceCode() const = 0;

    virtual int GetUniformBufferCount() const = 0;

    virtual SDL_GPUShaderStage GetSDLShaderStage() const = 0;

    virtual shaderc_shader_kind GetShaderKind() const = 0;
};
