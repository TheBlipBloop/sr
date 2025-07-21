// Example shader.
// Uniforms:
// float iTime         // Time since shader loaded (seconds).
// float iFrame        // Time since shader loaded (elapsed frames)
// vec2 iResolution    // Dimension of the viewport (pixels).
// vec2 iMouse         // Location of the mouse (pixels).

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord / iResolution.x);
    vec3 color = vec3(uv, 0.0);

    fragColor = vec4(color, 1.0);
}
