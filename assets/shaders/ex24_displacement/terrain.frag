#version 330 core

in Varyings {
    vec4 color;
    vec2 tex_coord;
    // We recieved the terrain height since we will use it to mix the terrain color based on the height.
    float terrain_height;
} fsin;

// We can use this uniform to tint the terrain.
uniform vec4 tint;
// We use two sampler; one for the terrain top colors (mountain tops) and another for the terrian bottom color (sea level).
uniform sampler2D terrain_top_sampler;
uniform sampler2D terrain_bottom_sampler;
// This contain 2 floats that define the threshold range of the terrain height.
// Any height below the x-component will be given the bottom color.
// Any height above the y-component will be given the top color.
// Heights in between will get a smooth interpolation between the 2 values.
uniform vec2 terrain_color_threshold;

out vec4 frag_color;

void main() {
    // First, we sample the 2 textures.
    vec4 top_color = texture(terrain_top_sampler, fsin.tex_coord);
    vec4 bottom_color = texture(terrain_bottom_sampler, fsin.tex_coord);
    // Then we decide the factor to use for mixing the color using smoothstep.
    float terrain_color_mix_factor = smoothstep(terrain_color_threshold.x, terrain_color_threshold.y, fsin.terrain_height);
    // Finally, we mix the colors and output the results after applying the per-vertex color and the tint.
    frag_color = tint * fsin.color * mix(bottom_color, top_color, terrain_color_mix_factor);
}
