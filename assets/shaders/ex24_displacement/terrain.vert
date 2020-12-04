#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;

uniform mat4 transform;
// This sampler will be used to read the terrain height as a certain coordinate
uniform sampler2D height_sampler;
// Since the terrain height texture coordinates and the terrain color texture coordinates have different scales,
// we use this uniform to tile the color texture while keeping the height texture at its original scale
// In other words, the height textures spans across the terrian only once so the texture coordinates should span [0, 1]
// On the other hand, we would like to have the color texture repeat multiple times (N times for example) so the texture coordinates should span [0, N]
// So instead of sending multiple texture coordinates with the attributes, we just send N and multiply the texture coordinates with its value.
uniform float texture_tiling;

out Varyings {
    vec4 color;
    vec2 tex_coord;
    // We send the terrain height since we will use it to mix the terrain color based on the height.
    float terrain_height;
} vsout;

void main() {
    float height = texture(height_sampler, tex_coord).r; // First, we read the terrain height.
    gl_Position = transform * vec4(position + vec3(0, height, 0), 1.0); // Then we add it to the y-component of the local position.
    vsout.color = color;
    // The texture coordinates will be used to sample the color in the fragment shader so we will multiply by the tiling factor.
    vsout.tex_coord = tex_coord * texture_tiling;
    // We send the terrain height since we will use it to mix the terrain color based on the height.
    vsout.terrain_height = height;
}
