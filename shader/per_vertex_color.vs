/*
vertex shader의 경우 layout을 사용하여 attribute index 지정
vertex shader의 경우 gl_Position을, fragment shader의 경우 픽셀의 최종 색상값을 출력해야 함
vertex shader의 out 변수들은 Rasterization 과정을 거쳐 픽셀단위로 보간되어 fragment shader의 in 변수들로 입력
*/

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec4 vertexColor;

void main() {
    gl_Position = vec4(aPos, 1.0);
    vertexColor = vec4(aColor, 1.0);
}