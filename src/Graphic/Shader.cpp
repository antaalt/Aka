#include <Aka/Graphic/Shader.h>

#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/OS/Logger.h>

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

Shader::Ptr Shader::create(ShaderID vert, ShaderID frag, const std::vector<Attributes>& attributes)
{
	return GraphicBackend::createShader(vert, frag, attributes);
}

Shader::Ptr Shader::createGeometry(ShaderID vert, ShaderID frag, ShaderID geometry, const std::vector<Attributes>& attributes)
{
	return GraphicBackend::createShaderGeometry(vert, frag, geometry, attributes);
}

Shader::Ptr Shader::createCompute(ShaderID compute, const std::vector<Attributes>& attributes)
{
	return GraphicBackend::createShaderCompute(compute, attributes);
}

};