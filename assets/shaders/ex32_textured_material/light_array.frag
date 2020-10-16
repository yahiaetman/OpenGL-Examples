#version 330 core

#include "light_common.glsl"

in Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 world;
    vec3 view;
    vec3 normal;
} fsin;

#define TYPE_DIRECTIONAL    0
#define TYPE_POINT          1
#define TYPE_SPOT           2

struct Light {
    int type;
    vec3 color;

    vec3 position, direction;
    float attenuation_constant;
    float attenuation_linear;
    float attenuation_quadratic;
    float inner_angle, outer_angle;
};

struct SkyLight {
    vec3 top_color, middle_color, bottom_color;
};

#define MAX_LIGHT_COUNT 16

uniform TexturedMaterial material;
uniform Light lights[MAX_LIGHT_COUNT];
uniform int light_count;
uniform SkyLight sky_light;

out vec4 frag_color;

void main() {
    Material sampled = sample_material(material, fsin.tex_coord);

    vec3 normal = normalize(fsin.normal);
    vec3 view = normalize(fsin.view);

    vec3 ambient = sampled.ambient * (normal.y > 0 ?
        mix(sky_light.middle_color, sky_light.top_color, normal.y) :
        mix(sky_light.middle_color, sky_light.bottom_color, -normal.y));

    vec3 accumulated_light = sampled.emissive + ambient;

    int count = min(light_count, MAX_LIGHT_COUNT);
    for(int index = 0; index < count; index++){
        Light light = lights[index];
        vec3 light_direction;
        float attenuation = 1;
        if(light.type == TYPE_DIRECTIONAL)
            light_direction = light.direction;
        else {
            light_direction = fsin.world - light.position;
            float distance = length(light_direction);
            light_direction /= distance;

            attenuation *= 1.0f / (light.attenuation_constant +
            light.attenuation_linear * distance +
            light.attenuation_quadratic * distance * distance);

            if(light.type == TYPE_SPOT){
                float angle = acos(dot(light.direction, light_direction));
                attenuation *= smoothstep(light.outer_angle, light.inner_angle, angle);
            }
        }

        vec3 diffuse = sampled.diffuse * light.color * calculate_lambert(normal, light_direction);
        vec3 specular = sampled.specular * light.color * calculate_phong(normal, light_direction, view, sampled.shininess);

        accumulated_light += (diffuse + specular) * attenuation;
    }

    frag_color = fsin.color * vec4(accumulated_light, 1.0f);
}
