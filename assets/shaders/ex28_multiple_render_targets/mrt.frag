#version 330 core

in Varyings {
    vec4 color;
    vec2 tex_coord;
} fsin;

uniform vec4 tint;
uniform sampler2D sampler;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec2 tex_coord;
layout(location = 2) out vec2 tex_coord_derivative;

void main() {
    frag_color = tint * fsin.color * texture(sampler, fsin.tex_coord);
    tex_coord = fract(fsin.tex_coord);
    tex_coord_derivative = fwidth(fsin.tex_coord);
}
