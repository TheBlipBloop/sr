#version 450

layout(location = 0) out vec4 colorOut;
layout(set = 0, binding = 0) uniform Block {
    float iTime;
} block;

void main()
{
    vec2 uv = ((gl_FragCoord.xy / 256) - 0.5) * 2.0; // Scale UV's to -1, 1

    colorOut = vec4(uv, block.iTime, 1.0);
}
