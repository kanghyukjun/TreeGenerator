#version 330 core
layout (location = 0) in vec3 aPos; // 0번째 attribute가 정점의 위치

void main() {
	gl_Position = vec4(aPos, 1.0); // vec3를 vec4 생성자에 사용
}