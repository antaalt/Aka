#include <Aka/Graphic/Shader.h>

#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/OS/Logger.h>

namespace aka {

Shader::Shader()
{
}

Shader::~Shader()
{
}

Shader::Ptr Shader::compile(const char* content, ShaderType type)
{
	return GraphicBackend::compile(content, type);
}

};