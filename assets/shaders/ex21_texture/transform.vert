#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;

uniform mat4 transform;

out Varyings {
    vec4 color;
    vec2 tex_coord;
} vsout;

void main() {
    gl_Position = transform * vec4(position, 1.0);
    vsout.color = color;
    vsout.tex_coord = tex_coord;
}
