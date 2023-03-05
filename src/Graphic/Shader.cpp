#include <Aka/Graphic/Shader.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

Shader::Shader(const char* name, ShaderType type) : 
	Resource(name, ResourceType::Shader),
	type(type)
{
}

ShaderMask getShaderMask(ShaderType type)
{
	if (type > ShaderType::Count)
		return ShaderMask::None;
	return ShaderMask(1 << EnumToIntegral(type));
}

ShaderHandle Shader::create(ShaderType type, const void* content, size_t size)
{
	return Application::app()->graphic()->createShader(type, content, size);
}

void Shader::destroy(ShaderHandle shader)
{
	Application::app()->graphic()->destroy(shader);
}

};
};