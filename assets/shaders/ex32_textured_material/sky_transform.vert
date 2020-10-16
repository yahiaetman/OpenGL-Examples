#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 view_projection;
uniform vec3 camera_position;

out Varyings {
    vec3 view;
} vsout;

void main() {
    vsout.view = position;
    vec4 clip_space = view_projection * vec4(position + camera_position, 1.0);
    gl_Position = clip_space.xyww;
}
