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

// This will include all the data we need about directional lights.
struct DirectionalLight {
    // These defines the colors and intensities of the light.
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;

    // Directional light are only defined by a direction. (It has no position).
    vec3 direction;
};

// Receive the material and the light as uniforms.
uniform Material material;
uniform DirectionalLight light;

out vec4 frag_color;

void main() {
    // First we normalize the normal and the view.
    vec3 normal = normalize(fsin.normal); // Although the normal was already normalized, it may become shorter during interpolation.
    vec3 view = normalize(fsin.view);

    // Now we compute the 3 components of the light separately.
    vec3 diffuse = material.diffuse * light.diffuse * calculate_lambert(normal, light.direction);
    vec3 specular = material.specular * light.specular * calculate_phong(normal, light.direction, view, material.shininess);
    vec3 ambient = material.ambient * light.ambient;

    // Then we combine the light component additively.
    frag_color = fsin.color * vec4(diffuse + specular + ambient, 1.0f);
}
