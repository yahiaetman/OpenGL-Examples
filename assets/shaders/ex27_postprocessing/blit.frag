#version 330 core

in vec2 tex_coord;

uniform sampler2D color_sampler;

out vec4 frag_color;

void main() {
    ivec2 texture_size = textureSize(color_sampler, 0);
    ivec2 frag_coord = ivec2(tex_coord * texture_size);
    frag_color = texelFetch(color_sampler, frag_coord, 0);
}
