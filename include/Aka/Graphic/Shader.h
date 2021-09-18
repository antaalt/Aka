#pragma once

#include <vector>

#include <Aka/Core/Geometry.h>
#include <Aka/Core/StrictType.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Mesh.h>

namespace aka {

enum class ShaderType
{
	Vertex          = (1 << 0),
	Fragment        = (1 << 1),
	Compute         = (1 << 2),
	Geometry        = (1 << 3),
	//TessControl     = (1 << 4),
	//TessEvaluation  = (1 << 5),
};

enum class UniformType 
{
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
	Buffer,
};

struct Uniform
{
	String name; // Name of uniform
	uint32_t binding; // Binding point of the uniform
	UniformType type; // Type of uniform
	ShaderType shaderType; // Shader stage used
	uint32_t count; // Number of element for this uniform
};

using ShaderHandle = StrictType<uintptr_t, struct ShaderStrictType>;

class Shader
{
public:
	using Ptr = std::shared_ptr<Shader>;
protected:
	Shader();
	Shader(const VertexAttribute* attributes, size_t count);
	Shader(const Shader&) = delete;
	const Shader& operator=(const Shader&) = delete;
	virtual ~Shader();
public:
	// Compile a shader
	static ShaderHandle compile(const char* content, ShaderType type);
	// Destroy a shader
	static void destroy(ShaderHandle shader);
public:
	// Create a vertex program
	static Shader::Ptr createVertexProgram(ShaderHandle vert, ShaderHandle frag, const VertexAttribute* attributes, size_t count);
	// Create a vertex program with geometry stage
	static Shader::Ptr createGeometryProgram(ShaderHandle vert, ShaderHandle frag, ShaderHandle geometry, const VertexAttribute* attributes, size_t count);
	// Create a compute program
	static Shader::Ptr createComputeProgram(ShaderHandle compute);
public:
	// Get uniform
	const Uniform* getUniform(const char* name) const;
	// Iterate uniforms
	std::vector<Uniform>::iterator begin();
	std::vector<Uniform>::iterator end();
	std::vector<Uniform>::const_iterator begin() const;
	std::vector<Uniform>::const_iterator end() const;
public:
	uint32_t getAttributeCount() const;
	const VertexAttribute& getAttribute(uint32_t iBinding) const;
protected:
	std::vector<Uniform> m_uniforms;
	std::vector<VertexAttribute> m_attributes;
};



}