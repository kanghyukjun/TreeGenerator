#version 330 core

uniform vec4 color; // 일종의 전역변수, 외부에서 값을 입력해줄 수 있다
out vec4 fragColor; // 최종 출력 색상

void main() {
    fragColor = color;
}