#version 330 core

in Varyings {
    vec4 color;
    vec2 tex_coord;
    float terrain_height;
} fsin;

uniform vec4 tint;
uniform sampler2D terrain_top_sampler;
uniform sampler2D terrain_bottom_sampler;
uniform vec2 terrain_color_threshold;

out vec4 frag_color;

void main() {
    vec4 top_color = texture(terrain_top_sampler, fsin.tex_coord);
    vec4 bottom_color = texture(terrain_bottom_sampler, fsin.tex_coord);
    float terrain_color_mix_factor = smoothstep(terrain_color_threshold.x, terrain_color_threshold.y, fsin.terrain_height);
    frag_color = tint * fsin.color * mix(bottom_color, top_color, terrain_color_mix_factor);
}
