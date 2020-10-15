#version 330 core

in vec2 tex_coord;

uniform sampler2D color_sampler;
uniform sampler2D depth_sampler;

uniform mat4 inverse_projection;

uniform vec3 fog_color;
uniform float fog_power;
uniform float fog_exponent;

out vec4 frag_color;

void main() {
    float depth = texture(depth_sampler, tex_coord).r;
    vec4 ndc_position = vec4(2.0f * tex_coord - 1.0f, 2.0f * depth - 1.0f, 1.0f);
    vec4 view_position = inverse_projection * ndc_position;
    float distance = length(view_position.xyz / view_position.w);
    float fog_mix_factor = fog_power * (1.0f - exp(- distance * fog_exponent));
    vec4 color = texture(color_sampler, tex_coord);
    color.rgb = mix(color.rgb, fog_color, fog_mix_factor);
    frag_color = color;
}
