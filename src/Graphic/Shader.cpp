#include <Aka/Graphic/Shader.h>

#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/OS/Logger.h>

namespace aka {

Shader::Shader(const VertexAttribute* attributes, size_t count) :
	m_attributes(attributes, attributes + count),
	m_valid(false)
{
}

Shader::~Shader()
{
}

ShaderID Shader::compile(const std::string& content, ShaderType type)
{
	return GraphicBackend::compile(content.c_str(), type);
}

ShaderID Shader::compile(const char* content, ShaderType type)
{
	return GraphicBackend::compile(content, type);
}

Shader::Ptr Shader::create(ShaderID vert, ShaderID frag, const VertexAttribute* attributes, size_t count)
{
	return GraphicBackend::createShader(vert, frag, attributes, count);
}

Shader::Ptr Shader::createGeometry(ShaderID vert, ShaderID frag, ShaderID geometry, const VertexAttribute* attributes, size_t count)
{
	return GraphicBackend::createShaderGeometry(vert, frag, geometry, attributes, count);
}

Shader::Ptr Shader::createCompute(ShaderID compute, const VertexAttribute* attributes, size_t count)
{
	return GraphicBackend::createShaderCompute(compute, attributes, count);
}

uint32_t Shader::getAttributeCount() const
{
	return static_cast<uint32_t>(m_attributes.size());
}

const VertexAttribute& Shader::getAttribute(uint32_t iBinding) const
{
	return m_attributes[iBinding];
}

};