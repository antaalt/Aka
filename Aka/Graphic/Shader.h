#pragma once

#include <vector>
#include <map>

#include "../Core/Geometry.h"
#include "../Core/StrictType.h"

namespace aka {

enum class ShaderType {
	Vertex,
	Fragment,
	Compute,
	Geometry,
	TesselationControl,
	TesselationEvaluation,
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

using ShaderID = StrictType<uint32_t, struct ShaderStrictType>;
using ProgramID = StrictType<uint32_t, struct ProgramStrictType>;
using UniformID = StrictType<int32_t, struct UniformStrictType>;

struct Uniform {
	UniformType type; // type of uniform (unused)
	ShaderType shaderType; // attached shader (unused)
	std::string name; // name of uniform
};

struct Shader
{
	using Ptr = std::shared_ptr<Shader>;

	Shader();
	Shader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Uniform>& uniforms);
	Shader(const Shader&) = delete;
	const Shader& operator=(const Shader&) = delete;
	~Shader();

	static ShaderID compile(const std::string &content, ShaderType type);
	static ShaderID compile(const char* content, ShaderType type);

	static Shader::Ptr create(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Uniform>& uniforms);

	void use();

	ProgramID id() const { return m_programID; }

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