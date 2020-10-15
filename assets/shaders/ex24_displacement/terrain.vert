#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;

uniform mat4 transform;
uniform sampler2D height_sampler;
uniform float texture_tiling;

out Varyings {
    vec4 color;
    vec2 tex_coord;
    float terrain_height;
} vsout;

void main() {
    float height = texture(height_sampler, tex_coord).r;
    gl_Position = transform * vec4(position + vec3(0, height, 0), 1.0);
    vsout.color = color;
    vsout.tex_coord = tex_coord * texture_tiling;
    vsout.terrain_height = height;
}
