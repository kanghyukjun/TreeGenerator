#include "common.h"
#include <fstream>
#include <sstream>

std::optional<std::string> LoadTextFile(const std::string& filename) {
	std::ifstream fin(filename);
	if (!fin.is_open()) {
		SPDLOG_ERROR("failed to open file: {}", filename);
		// optional 타입에 return할 것이 없는 경우 {} 사용
		return {};
	}
	std::stringstream text;
	text << fin.rdbuf();
	return text.str();
}