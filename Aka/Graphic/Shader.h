#pragma once

#include <vector>

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

using ShaderID = StrictType<uintptr_t, struct ShaderStrictType>;
using ProgramID = StrictType<uintptr_t, struct ProgramStrictType>;
using UniformID = StrictType<uintptr_t, struct UniformStrictType>;
using AttributeID = StrictType<uint32_t, struct UniformStrictType>;

struct Uniform {
	UniformID id;
	UniformType type; // type of uniform
	ShaderType shaderType; // attached shader
	uint32_t bufferIndex;
	uint32_t arrayLength;
	std::string name; // name of uniform
};

struct Attributes {
	AttributeID id;
	std::string name;
};

class Shader
{
public:
	using Ptr = std::shared_ptr<Shader>;
protected:
	Shader(const std::vector<Attributes>& attributes);
	Shader(const Shader&) = delete;
	const Shader& operator=(const Shader&) = delete;
	virtual ~Shader();
public:
	static ShaderID compile(const std::string &content, ShaderType type);
	static ShaderID compile(const char* content, ShaderType type);

	static Shader::Ptr create(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes);

	virtual void use() = 0;

	UniformID getUniformID(const char* name);
	const Uniform *getUniform(const char* name) const;

	template <typename T>
	void set(const char* name, T value);

	virtual void setFloat1(const char* name, float value) {}
	virtual void setFloat2(const char* name, float x, float y) {}
	virtual void setFloat3(const char* name, float x, float y, float z) {}
	virtual void setFloat4(const char* name, float x, float y, float z, float w) {}
	virtual void setUint1(const char* name, uint32_t value) {}
	virtual void setUint2(const char* name, uint32_t x, uint32_t y) {}
	virtual void setUint3(const char* name, uint32_t x, uint32_t y, uint32_t z) {}
	virtual void setUint4(const char* name, uint32_t x, uint32_t y, uint32_t z, uint32_t w) {}
	virtual void setInt1(const char* name, int32_t value) {}
	virtual void setInt2(const char* name, int32_t x, int32_t y) {}
	virtual void setInt3(const char* name, int32_t x, int32_t y, int32_t z) {}
	virtual void setInt4(const char* name, int32_t x, int32_t y, int32_t z, int32_t w) {}
	virtual void setMatrix4(const char* name, const float *data, bool transpose = false) {}
protected:
	std::vector<Uniform> m_uniforms;
	std::vector<Attributes> m_attributes;
};

}