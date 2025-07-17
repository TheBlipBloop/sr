#version 450

layout(location = 0) out vec4 colorOut;

void main()
{
    vec2 coords = gl_FragCoord.xy / 2500;
    colorOut = vec4(coords, 0.0, 1.0);
}
