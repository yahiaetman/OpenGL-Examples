#version 330 core

in Varyings {
    vec4 color;
    vec2 tex_coord;
} fsin;

uniform vec4 tint;
uniform sampler2D sampler;

out vec4 frag_color;

void main() {
    frag_color = tint * fsin.color * texture(sampler, fsin.tex_coord);
}
