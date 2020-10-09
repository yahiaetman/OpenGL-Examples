#version 330 core

in vec4 vertex_color;

uniform vec4 tint;

out vec4 frag_color;

void main() {
    frag_color = tint * vertex_color;
}
