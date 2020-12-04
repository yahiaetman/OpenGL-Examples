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

// This will include all the data we need about point lights.
struct PointLight {
    // These defines the colors and intensities of the light.
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;

    // Point lights only has a position. (No direction since it spreads in all directions).
    vec3 position;
    // The attenuation is used to control how the light dims out as we go further from it.
    float attenuation_constant;
    float attenuation_linear;
    float attenuation_quadratic;
};

// Receive the material and the light as uniforms.
uniform Material material;
uniform PointLight light;

out vec4 frag_color;

void main() {
    // First we normalize the normal and the view.
    vec3 normal = normalize(fsin.normal); // Although the normal was already normalized, it may become shorter during interpolation.
    vec3 view = normalize(fsin.view);

    // Then we get the light direction and distance relative to the pixel location in the world space.
    vec3 light_direction = fsin.world - light.position;
    float distance = length(light_direction);
    light_direction /= distance;

    // Then we calculate the attenuation factor based on the light distance from the pixel.
    float attenuation = 1.0f / (light.attenuation_constant +
                                light.attenuation_linear * distance +
                                light.attenuation_quadratic * distance * distance);

    // Now we compute the 3 components of the light separately.
    vec3 diffuse = material.diffuse * light.diffuse * calculate_lambert(normal, light_direction);
    vec3 specular = material.specular * light.specular * calculate_phong(normal, light_direction, view, material.shininess);
    vec3 ambient = material.ambient * light.ambient;

    // Then we combine the light component additively.
    // Note how the attenuation only affects the diffuse and speculat since ambient should be constant regardless of the position and direction.
    frag_color = fsin.color * vec4((diffuse + specular) * attenuation + ambient, 1.0f);
}
