#version 330 core

// The attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

// A transformation matrix sent as a Uniform
uniform mat4 transform;

// The varying
out vec4 vertex_color;

void main() {
    // To apply the transformation, we just multiply
    gl_Position = transform * vec4(position, 1.0);
    vertex_color = color;
}

// NOTE: Since "transform" is allowed to modify "w", we can create a matrix that applies perspective projection which is a key factor of realistic 3D visuals.