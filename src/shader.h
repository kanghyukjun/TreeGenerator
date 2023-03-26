#ifndef __SHADER_H__
#define __SHADER_H__

#include "common.h"

CLASS_PTR(Shader);
//	class Shader;
//	using ShaderUPtr = std::unique_ptr<Shader>;
//	using ShaderPtr = std::shared_ptr<Shader>;
//	using ShaderWPtr = std::weak_ptr<Shader>;

class Shader {
public:
	static ShaderUPtr CreateFromFile(const std::string& filename, GLenum shaderType);
	
	~Shader();
	uint32_t Get() const { return m_shader; }

private:
	Shader() {}
	bool LoadFile(const std::string& filename, GLenum shaderType);
	uint32_t m_shader{ 0 }; // openGL 쉐이더의 id를 저장하는 변수
};

#endif // __SHADER_H__