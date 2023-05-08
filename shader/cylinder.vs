#version 330 core

layout (location = 0) in vec3 aPos;
// layout (location = 1) in mat4 aOffset;
layout (location = 2) in vec2 aTexCoord;
// out vec4 fColor;
out vec2 texCoord;

uniform mat4 transform;
// uniform vec3 color;

void main() {
    mat4 Offset = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0);
    // gl_Position = transform * Offset * vec4(aPos, 1.0);
    // gl_Position = transform * aOffset * vec4(aPos, 1.0);
    gl_Position = transform * vec4(aPos, 1.0);
    texCoord = aTexCoord;
    // fColor = vec4(color, 1.0);
}