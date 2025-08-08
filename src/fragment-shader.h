#pragma once

#include "shader.h"

#include <filesystem>
#include <string>

#include <SDL3/SDL_gpu.h>
#include <shaderc/shaderc.h>

// Fragment shader that is loaded from a file and preprocessed to support
// shadertoy features.
class ShadertoyFragmentShader : public Shader
{
    /************************************************************************************************/
    /** Constructor */

public:

    // Constructs a new instance of ShadertoyFragmentShader linked to a certain
    // system file.
    // @source_file_path -- path to the source file this shader will be
    // rendering.
    ShadertoyFragmentShader(const char* source_file_path);

    /************************************************************************************************/
    /** Shadertoy Fragment Shader */

private:

    // Raw source code placed before source files code prior to compilation.
    // Used to transparently add shadertoy macros.
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

    // Raw source code appended to any source files code before compilation.
    // Used to implement a shadertoy-like entry point.
    const std::string SOURCE_FOOTER = R"(
	void main()
	{
		vec4 color = vec4(0);
		vec2 coord = vec2(gl_FragCoord.x, iResolution.y - gl_FragCoord.y);
		mainImage(color, coord);
		outColor = color;
	}
	)";

    // Path to the source file this shader loads source code from.
    std::filesystem::path m_source_file;

protected:

    // Returns a string holding the source code of a GLSL-SPIRV shader.
    virtual std::string GetShaderSourceCode() const override;

    // Returns the number of uniform buffers in the code returned by
    // @GetShaderSourceCode().
    virtual int GetUniformBufferCount() const override;

    // Returns the shader stage of code returned by
    // @GetShaderSourceCode().
    virtual SDL_GPUShaderStage GetSDLShaderStage() const override;

    // Returns the type of shader the code returned by
    // @GetShaderSourceCode().
    virtual shaderc_shader_kind GetShaderKind() const override;
};
