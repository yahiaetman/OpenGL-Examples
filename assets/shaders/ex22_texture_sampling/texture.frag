#version 330 core

// Since we are now receiving multiple Varyings, it would be nice to pack them together in what is called an "Interface Block".
// "Varyings" is just a name for the block (we can choose any name as long as it is the same in the vertex shader).
// "vsout" is an instance name (we can make an array of the same block and pick any name we want).
// instance names can be different in the vertex shader and the blocks will still match and link together.
// Interface blocks are nice for organization.
in Varyings {
    vec4 color;
    vec2 tex_coord;
} fsin;

uniform vec4 tint;
uniform sampler2D sampler; // We need a sampler to sample from a texture

out vec4 frag_color;

void main() {
    // Unlike "texelFetch" which is queried by coordinates in the pixel space,
    // "texture" is queried by the coordinates in the texture space.
    // Also "texture" applies filtering (so it also selects a suitable Level-Of-Detail automatically).
    frag_color = tint * fsin.color * texture(sampler, fsin.tex_coord);

    // NOTE: (coordinates in pixel space) = (coordinates in texture space) * (texture size)
    // So texture space is convenient since it is independent of the texture size.
}
