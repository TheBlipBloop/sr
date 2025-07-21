// NOTES:

// Added height ^ 4 + scaling for unique effect.

/*************************************************************************************************/
/** SDF -- Shapes */

float noise1D(float position)
{
    return fract(sin(position * 151.0) * 115249.0);
}

float noise1D(int position)
{
    return noise1D(float(position) / 256.0);
}

float noise2D(vec2 position)
{
    return fract(sin(position.x * 151.0 + position.y * 181.0) * 115249.0);
}

float sphere(vec3 ray, float radius)
{
    return length(ray) - radius;
}

float box(vec3 ray, vec3 bounds)
{
    vec3 q = abs(ray) - bounds;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float plane(vec3 ray, float height)
{
    return ray.y - height;
}

/*************************************************************************************************/
/** SDF -- Operations */

float sdf_union(float sdf_a, float sdf_b)
{
    return min(sdf_a, sdf_b);
}

float sdf_union_smooth(float sdf_a, float sdf_b, float factor)
{
    float h = clamp(0.5 - 0.5 * (sdf_b - sdf_a) / factor, 0.0, 1.0);
    return mix(sdf_b, sdf_a, h) - (factor * h * (1.0 - h));
}

float sdf_subtraction_smooth(float sdf_a, float sdf_b, float factor)
{
    float h = clamp(0.5 - 0.5 * (sdf_b + sdf_a) / factor, 0.0, 1.0);
    return mix(sdf_b, -sdf_a, h) + (factor * h * (1.0 - h));
}

vec3 translate(vec3 a, vec3 b)
{
    return a - b;
}

/*************************************************************************************************/
/** SDF -- Shapes */

/*************************************************************************************************/
/** Scene */

// First pass, water layer

float scene(vec3 ray_position)
{
    float sdf = 10000000.0;
    float subscene_scale = 12.0;
    ray_position.xz = mod(ray_position.xz, subscene_scale) - (subscene_scale * 0.5);

    vec3 sphere_position = vec3(0);
    float scale = 6.0;
    float speed = 0.5;
    for (float i = 0.0; i < 64.0; i += 1.0)
    {
        float x = sin(i * 17.0 + (speed * iTime) + i * 0.3) * scale;
        float y = cos(i * 19.0 + (speed * iTime) + i * 7.0) * scale;
        sphere_position = vec3(x, 0.0, y);

        float radius = 0.75 + 0.5 * sin(iTime * 0.2 * speed + i * 7.53);

        float sphere = sphere(translate(ray_position, sphere_position), radius);
        sdf = sdf_union(sdf, sphere);
    }

    sdf = sdf_subtraction_smooth(sdf, plane(ray_position, -0.5), 1.5);

    return sdf;
}

// Second pass, boat,

vec3 normal_scene(vec3 point)
{
    float step = 0.015;
    float origin = scene(point);
    return vec3((origin - scene(point + vec3(step, 0, 0))) / step,
        (origin - scene(point + vec3(0, step, 0))) / step,
        (origin - scene(point + vec3(0, 0, step))) / step);
}

/*************************************************************************************************/
/** Shading */

vec3 get_camera_position()
{
    return vec3(6.0, 0.25 * sin(iTime * 0.2), iTime * 1.0);
}

vec3 get_ray_direction(vec2 uv)
{
    vec3 ray_direction = normalize(vec3(uv, 1.));
    return ray_direction;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = ((fragCoord.xy / iResolution.xy) - 0.5) * 2.0; // Scale UV's to -1, 1
    uv.x *= iResolution.x / iResolution.y; // Correct for aspect ratios
    uv.y *= -1.0;

    // Round UV for pixalization
    float pixel_resolution = 0.015;
    uv = round(uv / pixel_resolution) * pixel_resolution;

    // ROLL:
    float roll_theta = cos(iTime / 4.0) * 0.046;
    float uv_polar = atan(uv.y, uv.x) + roll_theta;
    uv = vec2(length(uv) * cos(uv_polar), length(uv) * sin(uv_polar));

    vec3 ray_origin = get_camera_position(); // Camera location
    vec3 ray_direction = get_ray_direction(uv); // Ray projection direction

    float ray_distance = 0.0; // Cumulative distance, incremented during march

    int i = 0;
    float step_distance = 0.0;
    vec3 ray_position;

    float step_distances[2];
    vec3 ray_positions[2];

    for (i = 0; i < 64; i++)
    {
        ray_position = ray_origin + (ray_direction * ray_distance);
        step_distance = scene(ray_position);

        ray_distance += step_distance;

        // min stop
        if (step_distance < 0.01 || step_distance > 64.0)
        {
            break;
        }
    }

    float brightness = clamp(1.0 - clamp(ray_distance, 0., 100.) / 75.0, 0., 1.);
    brightness = pow(brightness, 4.0);
    vec3 color = vec3(brightness);
    color *= vec3(0.11, 0.14, 0.29); // * (2.0 * ray_position.y + 2.5);

    #define STAR_COUNT 64
    #define STAR_DUST 128

    {
        for (int i = 0; i < STAR_COUNT; i++)
        {
            float base_y = noise1D(i * 7) * 1.20 + 0.1;
            vec2 position = vec2(noise1D(i * 17 + 5), base_y);
            float pan_speed = 0.001;
            position.y += sin(iTime * 0.1 + (0.3 * float(i))) * 0.05;
            position.x = mod(position.x + iTime * pan_speed, 1.0);
            vec2 star_position = vec2(position.x * 5.0 - 2.5, position.y * 0.9);

            vec3 star_color = 0.35 + 0.65 * vec3(noise1D(i), noise1D(i * 3), noise1D(i * 7));
            color += 0.007 / distance(uv, star_position) * star_color * pow(noise1D(i * 28), 2.0);
            // Reflection test:
            if (uv.y < 0.0)
            {
                float reflection_strength = 0.0035 / distance(vec2(uv.x + sin(uv.y * 80.0) * 0.012, 0.0), vec2(star_position.x, 0.071));
                float y = noise1D(i * 37);
                reflection_strength *= 1.50 - distance(y, uv.y * 3.0);

                color += reflection_strength * star_color * pow(noise1D(i * 28), 2.0);
            }
        }

        for (int i = 0; i < STAR_DUST; i++)
        {
            vec3 dust_color = 0.05 + 0.5 * vec3(noise1D(i), noise1D(i * i * 3), noise1D(i * 7));
            vec2 position = vec2(noise1D(i * 23 + 3), noise1D(i * 27 - 3));
            position *= vec2(6.0, 1.1);
            position += vec2(-3.0, 0.10);

            float bob_magnitude = 0.5 * noise1D(i * 3 + 92);
            float bob_freq = 0.05 * noise1D(i * 76);
            position.y += sin(iTime * bob_freq + (0.3 * float(i))) * bob_magnitude;
            position = round(position / pixel_resolution) * pixel_resolution;

            if (position.y < 0.06)
            {
                position.y = 0.06;
            }

            uv = round(uv / pixel_resolution) * pixel_resolution;

            if (position == uv)
            {
                color += dust_color * noise1D(i - 32) * 0.5;
            }
            if (position == uv * vec2(-1., 1.))
            {
                color += dust_color * noise1D(i - 32) * 0.5;
            }
        }
    }
    fragColor = vec4(color.x, color.y, color.z, 1.0);
}
