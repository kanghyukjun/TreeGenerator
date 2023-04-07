#version 330 core

out vec4 fragColor;

in vec3 normal;
in vec3 position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main() {
    vec3 I = normalize(position - cameraPos); // 눈으로 바라보는 벡터
    vec3 R = reflect(I, normalize(normal)); // reflection 벡터
    fragColor = vec4(texture(skybox, R).rgb, 1.0); // reflection 벡터에 닿은 텍스쳐의 컬러를 가져와 fragColor 결정
}