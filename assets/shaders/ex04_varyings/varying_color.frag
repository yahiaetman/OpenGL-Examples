#version 330 core

// This output is received from the vertex shader
// These are usually called "Varyings" since their value varys from one run to the other
// However, "Varyings" are *only* used to denote variables connecting between different shaders
in vec4 vertex_color;
// It is noteworthy that the vertex shader sends a value for each vertex only
// So to get a value for each fragment, the rasterizer interpolates the values using what is called "barycentric coordinates".
// barycentric coordinates are 3 values that define how near we are to each vertex of a triangle (and they always sum to 1).

// This is the output of the fragment shader
// Note that even though this value will vary on each execution of the shader, it is still *not* called a "Varying".
// That's because it is not passed between the vertex and the fragment shader
out vec4 frag_color;

void main() {
    // Just pass the varying to the output
    frag_color = vertex_color;
}
