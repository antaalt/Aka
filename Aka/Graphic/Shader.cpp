#include "Shader.h"

#include "GraphicBackend.h"
#include "../OS/Logger.h"

namespace aka {

Shader::Shader(const std::vector<Attributes>& attributes) :
	m_attributes(attributes),
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

Shader::Ptr Shader::create(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes)
{
	return GraphicBackend::createShader(vert, frag, compute, attributes);
}

};