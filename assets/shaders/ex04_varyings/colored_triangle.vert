#version 330 core

// This output will be sent to the fragment shader
// These are usually called "Varyings" since their value varys from one run to the other
// However, "Varyings" are *only* used to denote variables connecting between the vertex and the fragment shaders
out vec4 vertex_color;

void main() {
    // Array of triangle positions
    const vec3 positions[3] = vec3[3](
        vec3( 0.5, -0.5, 0.0),
        vec3(-0.5, -0.5, 0.0),
        vec3( 0.0,  0.5, 0.0)
    );

    // Array of colors
    const vec4 colors[3] = vec4[3](
        vec4(1.0, 0.0, 0.0, 1.0),
        vec4(0.0, 1.0, 0.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0)
    );

    // As usual, we will pick one position and send to "gl_Position"
    gl_Position = vec4(positions[gl_VertexID], 1.0);
    // Similarly, we'll pick a color and send it to a Varying
    vertex_color = colors[gl_VertexID];
}