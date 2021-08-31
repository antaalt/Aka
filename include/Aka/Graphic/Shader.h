#pragma once

#include <vector>

#include <Aka/Core/Geometry.h>
#include <Aka/Core/StrictType.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Mesh.h>

namespace aka {

enum class ShaderType {
	Vertex          = (1 << 0),
	Fragment        = (1 << 1),
	Compute         = (1 << 2),
	Geometry        = (1 << 3),
	TessControl    = (1 << 4),
	TessEvaluation = (1 << 5),
};

enum class UniformType {
	None,
	Float,
	Int,
	UnsignedInt,
	Vec2,
	Vec3,
	Vec4,
	Mat3,
	Mat4,
	Image2D,
	Texture2D,
	Texture2DMultisample,
	TextureCubemap,
	Sampler2D,
	SamplerCube,
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

class Shader
{
public:
	using Ptr = std::shared_ptr<Shader>;
protected:
	Shader(const VertexAttribute* attributes, size_t count);
	Shader(const Shader&) = delete;
	const Shader& operator=(const Shader&) = delete;
	virtual ~Shader();
public:
	static ShaderID compile(const std::string &content, ShaderType type);
	static ShaderID compile(const char* content, ShaderType type);

	static Shader::Ptr create(ShaderID vert, ShaderID frag, const VertexAttribute* attributes, size_t count);
	static Shader::Ptr createGeometry(ShaderID vert, ShaderID frag, ShaderID geometry, const VertexAttribute* attributes, size_t count);
	static Shader::Ptr createCompute(ShaderID compute, const VertexAttribute* attributes, size_t count);

	uint32_t getAttributeCount() const;
	const VertexAttribute& getAttribute(uint32_t iBinding) const;
	bool valid() const { return m_valid; }
protected:
	std::vector<VertexAttribute> m_attributes;
	bool m_valid;
};



}