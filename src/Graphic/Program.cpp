#include <Aka/Graphic/Program.h>

#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/OS/Logger.h>

namespace aka {

Program::Program() :
	m_attributes()
{
}
Program::Program(const VertexAttribute* attributes, size_t count) :
	m_attributes(attributes, attributes + count)
{
}

Program::~Program()
{
}

Program::Ptr Program::createVertexProgram(Shader::Ptr vert, Shader::Ptr frag, const VertexAttribute* attributes, size_t count)
{
	return GraphicBackend::createVertexProgram(vert, frag, attributes, count);
}

Program::Ptr Program::createGeometryProgram(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, const VertexAttribute* attributes, size_t count)
{
	return GraphicBackend::createGeometryProgram(vert, frag, geometry, attributes, count);
}

Program::Ptr Program::createComputeProgram(Shader::Ptr compute)
{
	return GraphicBackend::createComputeProgram(compute);
}

const Uniform* Program::getUniform(const char* name) const
{
	for (const Uniform& uniform : m_uniforms)
	{
		if (uniform.name == name)
			return &uniform;
	}
	return nullptr;
}

std::vector<Uniform>::iterator Program::begin()
{
	return m_uniforms.begin();
}

std::vector<Uniform>::iterator Program::end()
{
	return m_uniforms.end();
}

std::vector<Uniform>::const_iterator Program::begin() const
{
	return m_uniforms.begin();
}

std::vector<Uniform>::const_iterator Program::end() const
{
	return m_uniforms.end();
}

uint32_t Program::getAttributeCount() const
{
	return static_cast<uint32_t>(m_attributes.size());
}

const VertexAttribute& Program::getAttribute(uint32_t iBinding) const
{
	return m_attributes[iBinding];
}

};