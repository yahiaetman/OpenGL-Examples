#version 330 core

layout(location = 0) in vec3 position;

// The sky box will always follow the camera so there is no need for an object to world matrix since we will translate it using the camera position.
uniform mat4 view_projection;
uniform vec3 camera_position;

out Varyings {
    // To compute the sky color, we only need the view direction.
    vec3 view;
} vsout;

void main() {
    // The view direction will be the position in the local space.
    vsout.view = position;
    // Then we transform the position to the homogenous clip space.
    vec4 clip_space = view_projection * vec4(position + camera_position, 1.0);
    // We replace z with w since this makes sure that the depth will always be 1 after normalization. This means that the sky will always be farther than any other object in the scene.
    gl_Position = clip_space.xyww;
}
