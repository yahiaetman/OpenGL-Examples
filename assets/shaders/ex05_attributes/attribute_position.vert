#version 330 core

// This an input recieved from the vertex buffers and injected into the vertex shader.
// This type of variables is called "Attributes". Only vertex shaders can receive attributes.
// The "vertex array" object is what defines how the data is read from the vertex buffer and send to this attribute.
// In "layout(location = 0)", we request that this attribute is put into location 0.
// If we omit the "layout" specifier, OpenGL will automatically select a location for it.
layout(location = 0) in vec3 position;

void main() {
    // Just send the attribute value to "gl_Position".
    gl_Position = vec4(position, 1.0);
}