#version 330 core

in vec2 tex_coord;

// This will be used to sample a color from the off-screen framebuffer on which we draw our scene.
uniform sampler2D color_sampler;

out vec4 frag_color;

void main() {
    // Get the texture size at mip level 0.
    ivec2 texture_size = textureSize(color_sampler, 0);
    // Convert the screen-space texture coordinate to a pixel coordinate in the "color_sampler" texture.
    ivec2 frag_coord = ivec2(tex_coord * texture_size);
    // Read and output color using texelFetch. Note that we don't need to use texture since we know that the texture_size and screen size is the same
    // so the pixels in the "color_sampler" and the screen will map 1:1 so no need for the filtering overhead.
    frag_color = texelFetch(color_sampler, frag_coord, 0);
}
