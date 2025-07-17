#version 450

/*
layout(location = 0) out vec2 texcoords; // texcoords are in the normalized [0,1] range for the viewport-filling quad part of the triangle

void main()
{
    vec2 vertices[3] = vec2[3](vec2(-1, -1), vec2(3, -1), vec2(-1, 3));
    gl_Position = vec4(vertices[gl_VertexIndex], 0, 1);
    texcoords = 0.5 * gl_Position.xy + vec2(0.5);
}
*/
void main()
{
    vec2 positions[3] = vec2[](
            vec2(-1, -1),
            vec2(3, -1),
            vec2(-1, 3)
        );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
