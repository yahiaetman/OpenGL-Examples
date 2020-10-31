#version 330 core

in vec4 vertex_color;

uniform vec4 tint; // A tint is something we can use to modify colors

out vec4 frag_color;

void main() {
    // We use a uniform and a varying together to be able to have a unique color for each object while sharing the same vertex data.
    frag_color = tint * vertex_color;
}
