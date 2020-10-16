#version 330 core

in Varyings {
    vec3 view;
} fsin;

struct SkyLight {
    vec3 top_color, middle_color, bottom_color;
};

uniform SkyLight sky_light;
uniform float exposure;

out vec4 frag_color;

void main() {
    vec3 view = normalize(fsin.view);

    vec3 sky_color = exposure * (view.y > 0 ?
        mix(sky_light.middle_color, sky_light.top_color, view.y) :
        mix(sky_light.middle_color, sky_light.bottom_color, -view.y));

    frag_color = vec4(sky_color, 1.0f);
}
