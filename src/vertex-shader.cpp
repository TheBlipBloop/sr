#include "vertex-shader.h"

std::string VertexShader::GetShaderSourceCode() const
{
    return VERTEX_SHADER_SOURCE;
}

int VertexShader::GetUniformBufferCount() const { return 0; }

SDL_GPUShaderStage VertexShader::GetSDLShaderStage() const
{
    return SDL_GPUShaderStage::SDL_GPU_SHADERSTAGE_VERTEX;
}

shaderc_shader_kind VertexShader::GetShaderKind() const
{
    return shaderc_glsl_vertex_shader;
};
