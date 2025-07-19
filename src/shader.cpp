#include "shader.h"
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <cassert>
#include <filesystem>
#include <shaderc/shaderc.h>

Shader::Shader(string shaderSourcePath, ShaderStage shaderStage)
{
    SourceFilePath = path(shaderSourcePath);
    Stage = shaderStage;
}

SDL_GPUShader* Shader::Load(SDL_GPUDevice* forDevice)
{
    if (ShaderCache == nullptr)
    {
        ShaderCache = CompileShader(SourceFilePath, forDevice);
    }

    return ShaderCache;
}

SDL_GPUShader* Shader::CompileShader(path sourceFile, SDL_GPUDevice* onDevice)
{
    assert(exists(sourceFile));
    assert(onDevice != nullptr);

    size_t sourceSize;
    const char* source = (char*)SDL_LoadFile(sourceFile.c_str(), &sourceSize);

    char* shaderCode;
    size_t shaderCodeSize;

    const bool compileSuccess = CompileSourceToShaderCode(
        source, sourceSize, &shaderCode, &shaderCodeSize);

    SDL_free((void*)source);

    if (!compileSuccess)
    {
        return nullptr;
    }

    SDL_GPUShaderCreateInfo createInfo = {0};
    createInfo.code = (Uint8*)shaderCode;
    createInfo.code_size = shaderCodeSize;
    createInfo.entrypoint = GLSL_ENTRY_POINT;
    createInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    createInfo.stage = GetSDLShaderStage();

    SDL_GPUShader* shader = SDL_CreateGPUShader(onDevice, &createInfo);
    free(shaderCode);

    return shader;
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

shaderc_shader_kind Shader::GetShaderKind() const
{
    if (Stage == ShaderStage::Fragment)
    {
        return shaderc_glsl_fragment_shader;
    }

    return shaderc_glsl_vertex_shader;
}

SDL_GPUShaderStage Shader::GetSDLShaderStage() const
{
    if (Stage == ShaderStage::Fragment)
    {
        return SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT;
    }

    return SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX;
}
