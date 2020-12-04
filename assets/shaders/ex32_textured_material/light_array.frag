#version 330 core

// We include the common light functions and structures.
// Note that GLSL doesn't support "#include" by default but we the library "stb_include" to recursively include the files as a string preprocessing phase.
#include "light_common.glsl"

in Varyings {
    vec4 color;
    vec2 tex_coord;
    // We will need the vertex position in the world space,
    vec3 world;
    // the view vector (vertex to eye vector in the world space),
    vec3 view;
    // and the surface normal in the world space.
    vec3 normal;
} fsin;

// These type constants match their peers in the C++ code.
#define TYPE_DIRECTIONAL    0
#define TYPE_POINT          1
#define TYPE_SPOT           2

// Now we will use a single struct for all light types.
struct Light {
    // This will hold the light type.
    int type;
    // This defines the color and intensity of the light.
    // Note that we no longer define different values for the diffuse and the specular because it is unrealistic.
    // Also, we skipped the ambient and we will use a sky light instead.
    vec3 color;

    // Position is used for point and spot lights. Direction is used for directional and spot lights.
    vec3 position, direction;
    // Attentuation factors are used for point and spot lights.
    float attenuation_constant;
    float attenuation_linear;
    float attenuation_quadratic;
    // Cone angles are used for spot lights.
    float inner_angle, outer_angle;
};

// The sky light will allow us to vary the ambient light based on the surface normal which is slightly more realistic compared to constant ambient lighting.
struct SkyLight {
    vec3 top_color, middle_color, bottom_color;
};

// This will define the maximum number of lights we can receive.
#define MAX_LIGHT_COUNT 16

// Now we recieve the material, light array, the actual number of lights sent from the cpu and the sky light.
uniform TexturedMaterial material;
uniform Light lights[MAX_LIGHT_COUNT];
uniform int light_count;
uniform SkyLight sky_light;

out vec4 frag_color;

void main() {
    // First, we sample the material at the current pixel.
    Material sampled = sample_material(material, fsin.tex_coord);

    // Then we normalize the normal and the view. These are done once and reused for every light type.
    vec3 normal = normalize(fsin.normal); // Although the normal was already normalized, it may become shorter during interpolation.
    vec3 view = normalize(fsin.view);

    // We calcuate the ambient using the sky light and the surface normal.
    vec3 ambient = sampled.ambient * (normal.y > 0 ?
        mix(sky_light.middle_color, sky_light.top_color, normal.y) :
        mix(sky_light.middle_color, sky_light.bottom_color, -normal.y));

    // Initially the accumulated light will hold the ambient light and the emissive light (light coming out of the object).
    vec3 accumulated_light = sampled.emissive + ambient;

    // Make sure that the actual light count never exceeds the maximum light count.
    int count = min(light_count, MAX_LIGHT_COUNT);
    // Now we will loop over all the lights.
    for(int index = 0; index < count; index++){
        Light light = lights[index];
        vec3 light_direction;
        float attenuation = 1;
        if(light.type == TYPE_DIRECTIONAL)
            light_direction = light.direction; // If light is directional, use its direction as the light direction
        else {
            // If not directional, compute the direction from the position.
            light_direction = fsin.world - light.position;
            float distance = length(light_direction);
            light_direction /= distance;

            // And compute the attenuation.
            attenuation *= 1.0f / (light.attenuation_constant +
            light.attenuation_linear * distance +
            light.attenuation_quadratic * distance * distance);

            if(light.type == TYPE_SPOT){
                // If it is a spot light, comput the angle attenuation.
                float angle = acos(dot(light.direction, light_direction));
                attenuation *= smoothstep(light.outer_angle, light.inner_angle, angle);
            }
        }

        // Now we compute the 2 components of the light separately.
        vec3 diffuse = sampled.diffuse * light.color * calculate_lambert(normal, light_direction);
        vec3 specular = sampled.specular * light.color * calculate_phong(normal, light_direction, view, sampled.shininess);

        // Then we accumulate the light components additively.
        accumulated_light += (diffuse + specular) * attenuation;
    }

    frag_color = fsin.color * vec4(accumulated_light, 1.0f);
}
