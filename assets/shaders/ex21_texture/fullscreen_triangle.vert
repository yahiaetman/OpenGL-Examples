#version 330 core

void main() {
    // This triangle will cover the whole screen (viewport)
    const vec2 positions[3] = vec2[3](
        vec2(-1, -1),
        vec2(3, -1),
        vec2(-1, 3)
    );

    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}
