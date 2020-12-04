#version 330 core

out vec2 tex_coord;

void main() {
    // A fullscreen triangle
    const vec2 positions[3] = vec2[3](
        vec2(-1, -1),
        vec2(3, -1),
        vec2(-1, 3)
    );

    // Get the triangle position based on the vertex ID.
    vec2 position = positions[gl_VertexID];
    gl_Position = vec4(position, 0.0, 1.0);
    // Convert the vertex position to a screen-space texture coordinate.
    tex_coord = (position + 1.0f) * 0.5f;
}
