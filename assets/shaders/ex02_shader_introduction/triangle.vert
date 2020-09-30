#version 330 core

void main() {
    const vec3 positions[] = {
        vec3( 0.5, -0.5, 0.0),
        vec3(-0.5, -0.5, 0.0),
        vec3( 0.0,  0.5, 0.0)
    };

    gl_Position = vec4(positions[gl_VertexID], 1.0);
}