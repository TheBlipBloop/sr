#pragma once

#include "shader.h"

#include <filesystem>
#include <string>

#include <SDL3/SDL_gpu.h>
#include <shaderc/shaderc.h>

class FragmentShader : public Shader
{
private:

    const std::string SOURCE_HEADER = R"(
	#version 450

	layout(location = 0) out vec4 outColor;

	layout(std140, set = 3, binding = 0) uniform UniformData{
		vec2 resolution;
		vec2 mouse_position;
		float time_seconds;
		int frame;
	} uniform_data;

	#define iTime uniform_data.time_seconds
	#define iResolution uniform_data.resolution
	#define iMouse vec2(uniform_data.mouse_position.x, uniform_data.resolution.y - uniform_data.mouse_position.y)
	#define iFrame uniform_data.frame
	)";

    const std::string SOURCE_FOOTER = R"(
	void main()
	{
		vec4 color = vec4(0);
		vec2 coord = vec2(gl_FragCoord.x, iResolution.y - gl_FragCoord.y);
		mainImage(color, coord);
		outColor = color;
	}
	)";

    std::filesystem::path m_source_file;

public:

    FragmentShader(const char* source_file_path);

protected:

    virtual std::string GetShaderSourceCode() const override;

    virtual int GetUniformBufferCount() const override;

    virtual SDL_GPUShaderStage GetSDLShaderStage() const override;

    virtual shaderc_shader_kind GetShaderKind() const override;
};
