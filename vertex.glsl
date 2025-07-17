#version 330 core

// Input: vertex ID
// layout(location = 0) in int gl_VertexID;

// Output to fragment shader
// out vec4 vColor;

void main()
{
    vec2 pos;
    if (gl_VertexIndex == 0)
    {
        pos = vec2(0.0, 0.5);
    }
    else if (gl_VertexIndex == 1)
    {
        pos = vec2(0.5, 0.5);
    }
    else // gl_VertexID == 2
    {
        pos = vec2(-0.5, 0.5);
    }

    gl_Position = vec4(pos, 0.0, 1.0);
}
