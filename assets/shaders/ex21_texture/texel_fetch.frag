#version 330 core

// To read colors from a texture, we need a "Sampler" which fetches the color from texture based on our query
uniform sampler2D sampler;
// The Level-Of-Detail (lod) allows us to select a mip level to read the color from.
// Higher LOD means smaller mip size, thus a lower resolution version of the texture.
uniform int lod;
// Since some mip levels are too small, we will use the zoom variable to zoom in.
uniform float zoom;

out vec4 frag_color;

void main() {
    // Get the mip level size based on the Level-Of-Detail (lod)
    // NOTE: the size of a mip level at a specific lod should be: full_texture_size / 2^lod
    ivec2 texture_size = textureSize(sampler, lod);
    // Get the pixel location on the framebuffer and apply zooming
    ivec2 frag_coord = ivec2(gl_FragCoord.xy / zoom);
    // Since we should query a color from outside the mip level range (behaviour is undefined),
    // We only read the colors if fragment coordinate is less than the mip level size
    if(all(lessThan(frag_coord, texture_size)))
        frag_color = texelFetch(sampler, frag_coord, lod); // Fetch a color from the texture at the given mip level (lod).
    else
        frag_color = vec4(0, 0, 0, 1); // Send black if fragment coordinate is out of range
}
