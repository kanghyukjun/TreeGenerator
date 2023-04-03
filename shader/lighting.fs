#version 330 core
in vec3 normal;
in vec2 texCoord;
in vec3 position;
out vec4 fragColor;

uniform vec3 viewPos;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;
 
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;

void main() {
    // ambient : 주변광, 물체가 기본적으로 받는 빛
    vec3 ambient = material.ambient * light.ambient;
    
    // diffuse : 분산광, 빛이 물체 표면에 부딛혔을 때 모든 방향으로 고르게 퍼지는 빛
    vec3 lightDir = normalize(light.position - position);
    vec3 pixelNorm = normalize(normal);
    float diff = max(dot(pixelNorm, lightDir), 0.0);
    vec3 diffuse = diff * material.diffuse * light.diffuse;
    
    // specular : 반사광, 빛이 물체 표면에 부딛혀 반사되는 광원
    vec3 viewDir = normalize(viewPos - position);
    vec3 reflectDir = reflect(-lightDir, pixelNorm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * material.specular * light.specular;
 
    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0);
}