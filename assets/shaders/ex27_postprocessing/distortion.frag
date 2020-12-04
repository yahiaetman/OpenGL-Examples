#version 330 core

in vec2 tex_coord;

// This will be used to sample a color from the off-screen framebuffer on which we draw our scene.
uniform sampler2D color_sampler;
// This will be used to distort our texture coordinates to get an underwater look.
uniform sampler2D distortion_sampler;
// This controls how much the texture coordinates will be distorted. 0 means no distortion.
uniform float distortion_power;

out vec4 frag_color;

void main() {
    // First, we sample a distortion value from the distortion texture, we then subtract 0.5 such that the values are centered around 0 then we apply the power.
    vec2 distortion = (texture(distortion_sampler, tex_coord).xy - 0.5f) * distortion_power;
    // Before sampling the scene color, we add the distortion to sample a color from a slightly different location thus causing the distorted look.
    frag_color = texture(color_sampler, tex_coord + distortion);
}
