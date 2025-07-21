#version 450

layout(location = 0) out vec4 colorOut;

void main()
{
    vec2 uv = ((gl_FragCoord.xy / 256) - 0.5) * 2.0; // Scale UV's to -1, 1
    vec3 color = vec3(sin(uv.x * 10 + uv.y * 5.0));

    color *= vec3(1.0, 0.5, 0.2);
    color += vec3(0.0, 0.0, pow(uv.y, 9.0));

    color /= uv.x;

    color += uv.y;

    colorOut = vec4(color, 1.0);
}
