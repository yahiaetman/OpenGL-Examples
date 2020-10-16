#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 projection;

out Varyings {
    vec3 view;
} vsout;

void main() {
    vsout.view = position;
    vec4 clip_space = projection * vec4(position, 1.0);
    gl_Position = clip_space.xyww;
}
