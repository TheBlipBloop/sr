# sr

## Overview

SR (Shader Renderer) is a small utility to help make GLSL fragment shader art. It aims to recreate the shadertoy experience locally by featuring convenient uniforms and hot-reloading.

![Image of SR rendering a shader while the shader is being developed.](docs/image_demo_0.png)

## Building

TODOC

## Usage

`sr [SHADER FILE]`

For example:

`sr mycoolshader.glsl`

## Shader Format

Shaders are expected to conform to Vulkan GLSL specs & implement the mainImage() function as shown below:

```
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
}
```

Shader files may access to the following uniforms:

```
float iTime         // Time since shader loaded (seconds).
float iFrame        // Time since shader loaded (elapsed frames)
vec2 iResolution    // Dimension of the viewport (pixels).
vec2 iMouse         // Location of the mouse (pixels).
```

Includes and buffers are not supported at this time.

## Note

SR is still very much work in progress & and may not perfectly replicate the shadertoy experience.
