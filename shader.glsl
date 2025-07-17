#version 450

layout(location = 0) out vec4 colorOut;

void main()
{
    //    vec2 coords = gl_FragCoord.xy / 2500;
    vec2 uv = ((gl_FragCoord.xy / 256) - 0.5) * 2.0; // Scale UV's to -1, 1

    colorOut = vec4(uv, 0.0, 1.0);
}
