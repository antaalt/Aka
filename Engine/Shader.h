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
	Sampler2D,
	SamplerBuffer
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

	static ShaderID create(const std::string &content, ShaderType type);
	static ShaderID create(const char* content, ShaderType type);

	void create(const ShaderInfo& info);
	void destroy();

	void use();

	ProgramID getID() const { return m_programID; }

	UniformID getUniformLocation(const char* name);

	template <typename T>
	void set(const char* name, T value);

	void setFloat1(const char* name, float value);
	void setFloat2(const char* name, float x, float y);
	void setFloat3(const char* name, float x, float y, float z);
	void setFloat4(const char* name, float x, float y, float z, float w);
	void setUint1(const char* name, uint32_t value);
	void setUint2(const char* name, uint32_t x, uint32_t y);
	void setUint3(const char* name, uint32_t x, uint32_t y, uint32_t z);
	void setUint4(const char* name, uint32_t x, uint32_t y, uint32_t z, uint32_t w);
	void setInt1(const char* name, int32_t value);
	void setInt2(const char* name, int32_t x, int32_t y);
	void setInt3(const char* name, int32_t x, int32_t y, int32_t z);
	void setInt4(const char* name, int32_t x, int32_t y, int32_t z, int32_t w);
	void setMatrix4(const char* name, const float *data, bool transpose = false);

private:
	ProgramID m_programID;
	std::map<std::string, UniformID> m_uniforms;
};

}