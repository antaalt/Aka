#pragma once

#include "GLBackend.h"

#include <string>
#include <stdexcept>
#include <map>

namespace app {
namespace gl {

struct Shader : public app::Shader {
	Shader();
	~Shader();

	static ShaderID create(const char* content, ShaderType type);
	static GLuint getType(ShaderType type);

	void create(const ShaderInfo& info) override;
	void destroy() override;

	void use() override;

	GLint getUniformLocation(const char* name);

	void setFloat1(const char* name, float value) override;
	void setFloat2(const char* name, const vec2& value) override;
	void setFloat3(const char* name, const vec3& value) override;
	void setFloat4(const char* name, const vec4& value) override;
	void setMat4(const char* name, const mat4& value) override;

private:
	GLuint id;
	std::map<std::string, GLint> uniforms;
};

};
};

