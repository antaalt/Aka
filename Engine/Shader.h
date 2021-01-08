#pragma once

#include <vector>
#include <map>

#include "Geometry.h"

namespace app {

enum class ShaderType {
	VERTEX_SHADER,
	TESS_CONTROL_SHADER,
	TESS_EVALUATION_SHADER,
	FRAGMENT_SHADER,
	GEOMETRY_SHADER,
	COMPUTE_SHADER
};

enum class UniformType {
	None,
	Vec,
	Vec2,
	Vec3,
	Vec4,
	Mat4,
	Texture2D,
	Sampler2D
};

using ShaderID = uint32_t; // TODO use strict type
using ProgramID = uint32_t; // TODO use strict type
using UniformID = int32_t;

struct Uniform {
	UniformType type; // type of uniform (unused)
	ShaderType shaderType; // attached shader (unused)
	std::string name; // name of uniform
};

struct ShaderInfo {
	ShaderID vertex;
	ShaderID frag;
	ShaderID compute;
	std::vector<Uniform> uniforms;
};

struct Shader {
	Shader();
	Shader(const Shader&) = delete;
	const Shader& operator=(const Shader&) = delete;
	~Shader();

	static ShaderID create(const char* content, ShaderType type);

	void create(const ShaderInfo& info);
	void destroy();

	void use();

	UniformID getUniformLocation(const char* name);

	template <typename T>
	void set(const char* name, T value);

	void setFloat1(const char* name, float value);
	void setFloat2(const char* name, const vec2f &value);
	void setFloat3(const char* name, const vec3f &value);
	void setFloat4(const char* name, const vec4f &value);
	void setMatrix4(const char* name, const mat4f &value, bool transpose = false);

private:
	ProgramID m_programID;
	std::map<std::string, UniformID> m_uniforms;
};

}