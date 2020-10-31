#version 330 core

// Here we are receiving 2 attributes
// We gave each of them a different location
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
// INFO: a location can only hold up to 4 floats or ints (a vec4 or a ivec4),
// so if we send a 4x4 matrix (mat4), it will use 4 locations.
// So this code is invalid:
//      layout(location = 0) in mat4 matrix;
//      layout(location = 1) in vec4 color;
// since "color" will overlap with "matrix". So we can modify it like this:
//      layout(location = 0) in mat4 matrix;
//      layout(location = 4) in vec4 color;
// Now it should work.
// Note that a 2x2 matrix has 4 floats only but it will still take 2 locations since each column will take a separate location.
// The same rule applies to array, so an array of N-floats will take N-locations (not N/4) since each float will take a whole location for itself.
// Since locations are limited, it is more efficient to try and pack data into 4D vectors.

// The varying for the vertex color
out vec4 vertex_color;

void main() {
    // Just passing data unmodified
    gl_Position = vec4(position, 1.0);
    vertex_color = color;
}