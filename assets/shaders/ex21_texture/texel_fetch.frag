#version 330 core

uniform sampler2D sampler;
uniform int lod;
uniform float zoom;

out vec4 frag_color;

void main() {
    ivec2 texture_size = textureSize(sampler, lod);
    ivec2 frag_coord = ivec2(gl_FragCoord.xy / zoom);
    if(all(lessThan(frag_coord, texture_size)))
        frag_color = texelFetch(sampler, frag_coord, lod);
    else
        frag_color = vec4(0, 0, 0, 1);
}
