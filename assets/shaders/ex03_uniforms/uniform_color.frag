#version 330 core

// Here we will define Uniform Variable
// they are called uniform because every run of the shader (under the same draw call) will see the same value
// so it is uniform across all executions of the shader
uniform vec3 color;
uniform float time;
uniform bool flicker;

// The output that goes to the frame buffer
out vec4 frag_color;

// Just a constant
#define PI 3.1415926535897932384626433832795

void main() {
    // Convert RGB to RGBA (in other words, add an alpha value).
    frag_color = vec4(color, 1.0);
    // If flickering, multiply it with a sinusoidal wave that oscillates over time
    if(flicker)
        frag_color.rgb *= 0.5 * (1 + cos(2 * PI * time));
}
