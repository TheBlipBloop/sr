#include "shader.h"
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <cassert>
#include <cstddef>
#include <shaderc/shaderc.h>

Shader::Shader()
{
    shaderCache = nullptr;
    deviceCache = nullptr;
}

Shader::~Shader()
{
    if (shaderCache != nullptr)
    {
        SDL_ReleaseGPUShader(deviceCache, shaderCache);
    }
}

SDL_GPUShader* Shader::Load(SDL_GPUDevice* forDevice, bool forceRegenerate)
{
    if (shaderCache == nullptr || forceRegenerate)
    {
        if (CompileShader(forDevice, &shaderCache))
        {
            deviceCache = forDevice;
        }
    }

    return shaderCache;
}

bool Shader::CompileShader(SDL_GPUDevice* onDevice, SDL_GPUShader** outShader)
{
    assert(onDevice != nullptr);

    string source = GetShaderSourceCode();

    char* shaderCode;
    size_t shaderCodeSize;

    const bool compileSuccess = CompileSourceToShaderCode(
        source.c_str(), source.size(), &shaderCode, &shaderCodeSize);

    if (!compileSuccess)
    {
        return false;
    }

    SDL_GPUShaderCreateInfo createInfo = {0};
    createInfo.code = (Uint8*)shaderCode;
    createInfo.code_size = shaderCodeSize;
    createInfo.entrypoint = GLSL_ENTRY_POINT;
    createInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    createInfo.stage = GetSDLShaderStage();
    createInfo.num_uniform_buffers = GetUniformBufferCount();

    *outShader = SDL_CreateGPUShader(onDevice, &createInfo);
    free(shaderCode);

    return true;
}

bool Shader::CompileSourceToShaderCode(const char* sourceText,
                                       size_t sourceTextLength,
                                       char** outShaderCode,
                                       size_t* outShaderCodeLength)
{
    shaderc_compiler_t glslCompiler = shaderc_compiler_initialize();

    shaderc_compilation_result_t result = shaderc_compile_into_spv(
        glslCompiler, sourceText, sourceTextLength, GetShaderKind(), "default",
        GLSL_ENTRY_POINT, nullptr);

    shaderc_compiler_release(glslCompiler);

    const auto status = shaderc_result_get_compilation_status(result);

    if (status != shaderc_compilation_status_success)
    {
        const size_t errors = shaderc_result_get_num_errors(result);

        SDL_Log("Failed to compile GLSL code. Encountered %zd errors.", errors);
        SDL_Log("%s", shaderc_result_get_error_message(result));

        shaderc_result_release(result);
        return false;
    }

    const char* bytes = shaderc_result_get_bytes(result);
    const size_t size = shaderc_result_get_length(result);

    *outShaderCode = (char*)malloc(size);
    memcpy(*outShaderCode, bytes, size);

    shaderc_result_release(result);

    *outShaderCodeLength = size;

    SDL_Log("Compiled shader of length : %zd", size);

    return true;
}
