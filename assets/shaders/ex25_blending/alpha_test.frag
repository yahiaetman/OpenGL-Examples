#version 330 core

in Varyings {
    vec4 color;
    vec2 tex_coord;
} fsin;

uniform vec4 tint;
uniform sampler2D sampler;
// We will use this threshold to decide which pixels to discard based on its alpha value.
uniform float alpha_threshold;

out vec4 frag_color;

void main() {
    vec4 color = tint * fsin.color * texture(sampler, fsin.tex_coord);
    // If the alpha value is below the threshold, discard the whole pixel.
    if(color.a < alpha_threshold) discard;
    frag_color = color;
}
