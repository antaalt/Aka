#pragma once

#include <fstream>
#include <vector>

#include "Geometry.h"

namespace app {

inline std::string loadFromFile(const char* path)
{
	std::ifstream ifs(path);
	if (!ifs)
		throw std::runtime_error("Could not load file " + std::string(path));
	return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}
inline std::vector<uint8_t> loadFromBinaryFile(const char* path)
{
	std::basic_ifstream<uint8_t> ifs(path);
	if (!ifs)
		throw std::runtime_error("Could not load file " + std::string(path));
	return std::vector<uint8_t>((std::istreambuf_iterator<uint8_t>(ifs)), (std::istreambuf_iterator<uint8_t>()));
}

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
	virtual void create(const ShaderInfo& info) = 0;
	virtual void destroy() = 0;

	virtual void use() = 0;

	template <typename T>
	void set(const char* name, T value);

	virtual void setFloat1(const char* name, float value) = 0;
	virtual void setFloat2(const char* name, const vec2f &value) = 0;
	virtual void setFloat3(const char* name, const vec3f &value) = 0;
	virtual void setFloat4(const char* name, const vec4f &value) = 0;
	virtual void setMat4(const char* name, const mat4f &value) = 0;
};

template <>
inline void Shader::set(const char* name, float value) {
	setFloat1(name, value);
}
template <>
inline void Shader::set(const char* name, vec2f value) {
	setFloat2(name, value);
}
template <>
inline void Shader::set(const char* name, vec3f value) {
	setFloat3(name, value);
}
template <>
inline void Shader::set(const char* name, vec4f value) {
	setFloat4(name, value);
}
template <>
inline void Shader::set(const char* name, mat4f value) {
	setMat4(name, value);
}

}