#version 330 core
// The first line of any shader must a '#version" command which defines the glsl version to use

// This will be the output (you can choose any name) and it will be sent to the frame buffer (back buffer of the window).
out vec4 frag_color;

// This is entry point of the fragment shader and it will be called for every fragment covered by the rasterized geometry
void main() {
    // Here we just output a constant color which is red (R=1, G=0, B=0, A=1)
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}
