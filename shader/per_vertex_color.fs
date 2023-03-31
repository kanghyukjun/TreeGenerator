/*
uniform variable을 사용하면 shader에서 사용할 수 있는 global 값을 설정할 수 있음
glGetShaderLocation()으로 uniform handle을 얻을 수 있음
program이 바인딩된 상태에서 glUniform...()으로 uniform variable 값 설정 가능
필요에 따라 정점 별로 여러 개의 attribute를 설정하여 사용 가능
*/

#version 330 core

in vec4 vertexColor;
out vec4 fragColor;

void main() {
    fragColor = vertexColor;
}