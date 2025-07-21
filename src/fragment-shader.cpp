#include "fragment-shader.h"

#include <SDL3/SDL_iostream.h>
#include <cstddef>

FragmentShader::FragmentShader(const char* source_file_path)
{
    m_source_file = source_file_path;
}

std::string FragmentShader::GetShaderSourceCode() const
{
    size_t source_size;
    std::string source =
        (char*)SDL_LoadFile(m_source_file.c_str(), &source_size);
    std::string processed_source = SOURCE_HEADER + source + SOURCE_FOOTER;
    return processed_source;
}

int FragmentShader::GetUniformBufferCount() const { return 1; }

SDL_GPUShaderStage FragmentShader::GetSDLShaderStage() const
{
    return SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_FRAGMENT;
}

shaderc_shader_kind FragmentShader::GetShaderKind() const
{
    return shaderc_glsl_fragment_shader;
};
