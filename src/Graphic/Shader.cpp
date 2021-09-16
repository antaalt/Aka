#include <Aka/Graphic/Shader.h>

#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/OS/Logger.h>

namespace aka {

Shader::Shader() :
	m_attributes()
{
}
Shader::Shader(const VertexAttribute* attributes, size_t count) :
	m_attributes(attributes, attributes + count)
{
}

Shader::~Shader()
{
}

ShaderHandle Shader::compile(const char* content, ShaderType type)
{
	return GraphicBackend::compile(content, type);
}

void Shader::destroy(ShaderHandle shader)
{
	return GraphicBackend::destroy(shader);
}

Shader::Ptr Shader::createVertexProgram(ShaderHandle vert, ShaderHandle frag, const VertexAttribute* attributes, size_t count)
{
	return GraphicBackend::createShader(vert, frag, attributes, count);
}

Shader::Ptr Shader::createGeometryProgram(ShaderHandle vert, ShaderHandle frag, ShaderHandle geometry, const VertexAttribute* attributes, size_t count)
{
	return GraphicBackend::createShaderGeometry(vert, frag, geometry, attributes, count);
}

Shader::Ptr Shader::createComputeProgram(ShaderHandle compute)
{
	return GraphicBackend::createShaderCompute(compute);
}

const Uniform* Shader::getUniform(const char* name) const
{
	for (const Uniform& uniform : m_uniforms)
	{
		if (uniform.name == name)
			return &uniform;
	}
	return nullptr;
}

std::vector<Uniform>::iterator Shader::begin()
{
	return m_uniforms.begin();
}

std::vector<Uniform>::iterator Shader::end()
{
	return m_uniforms.end();
}

std::vector<Uniform>::const_iterator Shader::begin() const
{
	return m_uniforms.begin();
}

std::vector<Uniform>::const_iterator Shader::end() const
{
	return m_uniforms.end();
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