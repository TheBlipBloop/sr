#pragma once

#include "shader.h"

#include <string>

#include <SDL3/SDL_gpu.h>
#include <shaderc/shaderc.h>

// Vertex shader for a triangle that spans the entire view port.
class VertexShader : public Shader
{
    const std::string VERTEX_SHADER_SOURCE = R"(
	#version 450

	void main()
	{
		vec2 positions[3] = vec2[](
				vec2(-1, -1),
				vec2(3, -1),
				vec2(-1, 3)
			);

		gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	}
	)";

    virtual std::string GetShaderSourceCode() const override;

    virtual int GetUniformBufferCount() const override;

    virtual SDL_GPUShaderStage GetSDLShaderStage() const override;

    virtual shaderc_shader_kind GetShaderKind() const override;
};
