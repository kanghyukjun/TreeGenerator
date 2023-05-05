#version 330 core

layout (location = 0) in vec3 aPos;
// layout (location = 1) in mat4 aOffset;
out vec4 fColor;

uniform mat4 transform;
uniform vec3 color;

void main() {
    mat4 Offset = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0);
    // gl_Position = transform * Offset * vec4(aPos, 1.0);
    // gl_Position = transform * aOffset * vec4(aPos, 1.0);
    gl_Position = transform * vec4(aPos, 1.0);
    fColor = vec4(color, 1.0);
}