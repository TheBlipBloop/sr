#include "shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_log.h>
#include <cassert>
#include <shaderc/shaderc.h>

Shader::Shader()
{
    m_shader_cache = nullptr;
    m_device_cache = nullptr;
}

Shader::~Shader()
{
    if (m_shader_cache != nullptr)
    {
        SDL_ReleaseGPUShader(m_device_cache, m_shader_cache);
    }
}

SDL_GPUShader* Shader::Load(SDL_GPUDevice* for_device, bool force_regenerate)
{
    if (m_shader_cache == nullptr)
    {
        if (CompileShader(for_device, &m_shader_cache))
        {
            m_device_cache = for_device;
        }
    }

    if (force_regenerate)
    {
        SDL_GPUShader* shader_next = nullptr;

        if (CompileShader(for_device, &shader_next))
        {
            if (m_shader_cache)
            {
                SDL_ReleaseGPUShader(m_device_cache, m_shader_cache);
            }

            m_shader_cache = shader_next;
            m_device_cache = for_device;
        }
    }

    return m_shader_cache;
}

bool Shader::CompileShader(SDL_GPUDevice* for_device,
                           SDL_GPUShader** out_shader)
{
    assert(for_device != nullptr);

    std::string source = GetShaderSourceCode();

    char* shader_code;
    size_t shader_code_size;

    const bool compile_success = CompileSourceToShaderCode(
        source.c_str(), source.size(), &shader_code, &shader_code_size);

    if (!compile_success)
    {
        return false;
    }

    SDL_GPUShaderCreateInfo create_info = {0};
    create_info.code = (Uint8*)shader_code;
    create_info.code_size = shader_code_size;
    create_info.entrypoint = GLSL_ENTRY_POINT.c_str();
    create_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
    create_info.stage = GetSDLShaderStage();
    create_info.num_uniform_buffers = GetUniformBufferCount();

    *out_shader = SDL_CreateGPUShader(for_device, &create_info);
    free(shader_code);

    return true;
}

bool Shader::CompileSourceToShaderCode(const char* source_text,
                                       size_t source_text_length,
                                       char** out_shader_code,
                                       size_t* out_shader_code_length)
{
    shaderc_compiler_t glsl_compiler = shaderc_compiler_initialize();

    shaderc_compilation_result_t result = shaderc_compile_into_spv(
        glsl_compiler, source_text, source_text_length, GetShaderKind(),
        "default", GLSL_ENTRY_POINT.c_str(), nullptr);

    shaderc_compiler_release(glsl_compiler);

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

    *out_shader_code = (char*)malloc(size);
    memcpy(*out_shader_code, bytes, size);

    shaderc_result_release(result);

    *out_shader_code_length = size;

    return true;
}
