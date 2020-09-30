#version 330 core

uniform vec3 color;
uniform float time;
uniform bool flicker;

out vec4 frag_color;

#define PI 3.1415926535897932384626433832795

void main() {
    frag_color = vec4(color, 1.0);
    if(flicker)
        frag_color.rgb *= 0.5 * (1 + cos(2 * PI * time));
}
