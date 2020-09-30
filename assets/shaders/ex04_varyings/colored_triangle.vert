#version 330 core

out vec4 vertex_color;

void main() {
    const vec3 positions[] = {
        vec3( 0.5, -0.5, 0.0),
        vec3(-0.5, -0.5, 0.0),
        vec3( 0.0,  0.5, 0.0)
    };

    const vec4 colors[] = {
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(0.0, 1.0, 0.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0)
    };

    gl_Position = vec4(positions[gl_VertexID], 1.0);
    vertex_color = colors[gl_VertexID];
}