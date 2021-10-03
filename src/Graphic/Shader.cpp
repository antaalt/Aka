#include <Aka/Graphic/Shader.h>

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Core/Application.h>
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
	return Application::graphic()->compile(content, type);
}

};