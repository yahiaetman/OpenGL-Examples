#version 330 core

#include "light_common.glsl"

in Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 world;
    vec3 view;
    vec3 normal;
} fsin;

struct PointLight {
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;

    vec3 position;
    float attenuation_constant;
    float attenuation_linear;
    float attenuation_quadratic;
};

uniform Material material;
uniform PointLight light;

out vec4 frag_color;

void main() {
    vec3 normal = normalize(fsin.normal);
    vec3 view = normalize(fsin.view);

    vec3 light_direction = fsin.world - light.position;
    float distance = length(light_direction);
    light_direction /= distance;

    float attenuation = 1.0f / (light.attenuation_constant +
                                light.attenuation_linear * distance +
                                light.attenuation_quadratic * distance * distance);

    vec3 diffuse = material.diffuse * light.diffuse * calculate_lambert(normal, light_direction);
    vec3 specular = material.specular * light.specular * calculate_phong(normal, light_direction, view, material.shininess);
    vec3 ambient = material.ambient * light.ambient;

    frag_color = fsin.color * vec4((diffuse + specular) * attenuation + ambient, 1.0f);
}
