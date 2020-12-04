#version 330 core

in Varyings {
    // To compute the sky color, we only need the view direction.
    vec3 view;
} fsin;

// These are the 3 colors of the sky.
struct SkyLight {
    vec3 top_color, middle_color, bottom_color;
};

uniform SkyLight sky_light;
uniform float exposure; // Exposure will be used to control how bright the sky will look.

out vec4 frag_color;

void main() {
    // First, we normalize the view direction.
    vec3 view = normalize(fsin.view);

    // Then we compute the sky color based on the view direction.
    vec3 sky_color = exposure * (view.y > 0 ?
        mix(sky_light.middle_color, sky_light.top_color, view.y) :
        mix(sky_light.middle_color, sky_light.bottom_color, -view.y));

    frag_color = vec4(sky_color, 1.0f);
}
