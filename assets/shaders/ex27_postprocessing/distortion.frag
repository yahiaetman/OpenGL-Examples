#version 330 core

in vec2 tex_coord;

uniform sampler2D color_sampler;
uniform sampler2D distortion_sampler;
uniform float distortion_power;

out vec4 frag_color;

void main() {
    vec2 distortion = (texture(distortion_sampler, tex_coord).xy - 0.5f) * distortion_power;
    frag_color = texture(color_sampler, tex_coord + distortion);
}
