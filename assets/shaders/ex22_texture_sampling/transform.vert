#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
// Now we are recieving a new attribute "Texture Coordinate" (tex_coord).
// This attribute will define where the vertex lies in the texture space
layout(location = 2) in vec2 tex_coord;

uniform mat4 transform;

// Since we are now sending multiple Varyings, it would be nice to pack them together in what is called an "Interface Block".
// "Varyings" is just a name for the block (we can choose any name as long as it is the same in the fragment shader).
// "vsout" is an instance name (we can make an array of the same block and pick any name we want).
// instance names can be different in the fragment shader and the blocks will still match and link together.
// Interface blocks are nice for organization.
out Varyings {
    vec4 color;
    vec2 tex_coord;
} vsout;

void main() {
    gl_Position = transform * vec4(position, 1.0);
    vsout.color = color;
    // The fragment shader will use the texture coordinates so we pass them to it.
    vsout.tex_coord = tex_coord;
}
