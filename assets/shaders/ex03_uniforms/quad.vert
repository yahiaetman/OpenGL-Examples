#version 330 core

// Here we will define Uniform Variable
// they are called uniform because every run of the shader (under the same draw call) will see the same value
// so it is uniform across all executions of the shader
uniform vec2 scale;
uniform vec2 translation;
uniform float time;
uniform bool vibrate;

// Just a constant
#define PI 3.1415926535897932384626433832795

void main() {
    // This time we have 6 points since we are drawing a Quad (in NDC, it is a square).
    const vec3 positions[6] = vec3[6](
        vec3(-1.0, -1.0, 0.0),
        vec3( 1.0, -1.0, 0.0),
        vec3( 1.0,  1.0, 0.0),
        vec3( 1.0,  1.0, 0.0),
        vec3(-1.0,  1.0, 0.0),
        vec3(-1.0, -1.0, 0.0)
    );

    // get the position based on the vertex id
    vec3 position = positions[gl_VertexID];
    // Scale it
    position.xy *= scale;
    // If vibrate is on, scale it with a sinusoidal wave that oscillates over time
    if(vibrate)
        position.xy *= 1 + 0.1 * sin(2 * PI * time);
    // Translate it
    position.xy += translation;

    // Finally send the value to "gl_Position".
    gl_Position = vec4(position, 1.0);
}