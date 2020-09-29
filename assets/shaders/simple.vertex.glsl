#version 330 core

layout(location = 0) in float3 position;

void main() {
    gl_Position = float4(position, 1.0f);
}
