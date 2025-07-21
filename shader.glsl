#version 450

layout(location = 0) out vec4 colorOut;

layout(std140, set = 3, binding = 0) uniform Block {
    float iTime;
} block;

void main()
{
    vec2 uv = ((gl_FragCoord.xy / 256) - 0.5) * 2.0; // Scale UV's to -1, 1
    vec3 color = vec3(sin(uv.x * 30 + uv.y * 5.0 + block.iTime));

    color *= vec3(1.0, 0.5, 0.2);
    color += vec3(0.0, 0.0, pow(uv.y, 2.0));

    color /= uv.x;

    color += uv.y;

    colorOut = vec4(color, 1.0);
}
