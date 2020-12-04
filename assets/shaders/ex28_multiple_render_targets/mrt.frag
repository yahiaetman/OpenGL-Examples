#version 330 core

in Varyings {
    vec4 color;
    vec2 tex_coord;
} fsin;

uniform vec4 tint;
uniform sampler2D sampler;

// Here we will output 3 values and each value will go to a certain render target.
layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec2 tex_coord;
layout(location = 2) out vec2 tex_coord_derivative;

void main() {
    // First output will be the texture color after applying the per-vertex color and tint.
    frag_color = tint * fsin.color * texture(sampler, fsin.tex_coord);
    // Second output will be the fractional part of the texture coordinates.
    tex_coord = fract(fsin.tex_coord);
    // Third output will be gradient length of the texture coordinates with respect to the screen pixels.
    tex_coord_derivative = fwidth(fsin.tex_coord);
}
