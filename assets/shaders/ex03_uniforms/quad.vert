#version 330 core

uniform vec2 scale;
uniform vec2 translation;
uniform float time;
uniform bool vibrate;

#define PI 3.1415926535897932384626433832795

void main() {
    const vec3 positions[] = {
        vec3(-1.0, -1.0, 0.0),
        vec3( 1.0, -1.0, 0.0),
        vec3( 1.0,  1.0, 0.0),
        vec3( 1.0,  1.0, 0.0),
        vec3(-1.0,  1.0, 0.0),
        vec3(-1.0, -1.0, 0.0)
    };

    vec3 position = positions[gl_VertexID];
    position.xy *= scale;
    if(vibrate)
        position.xy *= 1 + 0.1 * sin(2 * PI * time);
    position.xy += translation;

    gl_Position = vec4(position, 1.0);
}