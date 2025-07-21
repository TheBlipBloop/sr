#version 450

layout(location = 0) out vec4 colorOut;

layout(std140, set = 3, binding = 0) uniform Block {
    vec2 resolution;
    float time_seconds;
    int frame;
} block;

#define iTime block.time_seconds
#define iResolution block.resolution

/*
uniform vec2 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform float iFrame;
uniform float iChannelTime[4];
uniform vec4 iMouse;
uniform vec4 iDate;
uniform float iSampleRate;
uniform vec3 iChannelResolution[4];
uniform samplerXX iChanneli;
*/

void main()
{
    vec2 uv = ((gl_FragCoord.xy / iResolution.x) - 0.5) * 2.0; // Scale UV's to -1, 1
    vec3 color = vec3(sin(uv.x * 30 + uv.y * 5.0 + iTime));

    color *= vec3(1.0, 0.5, 0.2);
    color += vec3(0.0, 0.0, pow(uv.y, 2.0));

    color += uv.y / 5.0;

    colorOut = vec4(color, 1.0);
}
