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
		float time_seconds;
		vec2 mouse_position;
		int frame;
	} uniform_data;

	#define iTime uniform_data.time_seconds
	#define iResolution uniform_data.resolution
	#define iMouse uniform_data.mouse_position
	#define iFrame uniform_data.frame
	)";

    const std::string SOURCE_FOOTER = R"(
	void main()
	{
		vec4 color = vec4(0);
		mainImage(color, gl_FragCoord.xy);
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
