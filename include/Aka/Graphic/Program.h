#pragma once

#include <vector>

#include <Aka/Core/Geometry.h>
#include <Aka/Core/StrictType.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/Mesh.h>

namespace aka {

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

class Program
{
public:
	using Ptr = std::shared_ptr<Program>;
protected:
	Program();
	Program(const VertexAttribute* attributes, size_t count);
	Program(const Program&) = delete;
	const Program& operator=(const Program&) = delete;
	virtual ~Program();
public:
	// Create a vertex program
	static Program::Ptr createVertexProgram(Shader::Ptr vert, Shader::Ptr frag, const VertexAttribute* attributes, size_t count);
	// Create a vertex program with geometry 
	static Program::Ptr createGeometryProgram(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, const VertexAttribute* attributes, size_t count);
	// Create a compute program
	static Program::Ptr createComputeProgram(Shader::Ptr compute);
public:
	// Get uniform
	const Uniform* getUniform(const char* name) const;
	// Iterate uniforms
	std::vector<Uniform>::iterator begin();
	std::vector<Uniform>::iterator end();
	std::vector<Uniform>::const_iterator begin() const;
	std::vector<Uniform>::const_iterator end() const;
public:
	// Get number of attribute
	uint32_t getAttributeCount() const;
	// Get attribute at given binding
	const VertexAttribute& getAttribute(uint32_t iBinding) const;
protected:
	std::vector<Uniform> m_uniforms;
	std::vector<VertexAttribute> m_attributes;
};



}