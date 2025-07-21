#include "fragment-shader.h"
#include <SDL3/SDL_iostream.h>
#include <cstddef>
#include <shaderc/shaderc.h>

FragmentShader::FragmentShader(const char* sourceFilePath)
{
    sourceFile = sourceFilePath;
}

string FragmentShader::GetShaderSourceCode() const
{
    size_t sourceSize;
    string source = (char*)SDL_LoadFile(sourceFile.c_str(), &sourceSize);
    string processedSource = sourceHeader + source + sourceFooter;
    return processedSource;
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
