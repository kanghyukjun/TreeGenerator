#ifndef __COMMON_H__
#define __COMMON_H__

#include <memory>
#include <string>
#include <optional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// type aliasing
#define CLASS_PTR(klassName) \
class klassName; \
using klassName ## UPtr = std::unique_ptr<klassName>; \
using klassName ## Ptr = std::shared_ptr<klassName>; \
using klassName ## WPtr = std::weak_ptr<klassName>;

// optinal -> 파일을 읽지 못하는 경우를 포인터 없이 편리하게 사용 가능
std::optional<std::string> LoadTextFile(const std::string& filename);
glm::vec3 GetAttenuationCoeff(float distance);
float RandomRange(float minValue = 0.0f, float maxValue = 1.0f);

#endif // __COMMON_H__