#include <Aka/Graphic/Shader.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

Shader::Shader(const char* name, const char* entryPoint, ShaderType type) :
	Resource(name, ResourceType::Shader),
	type(type),
	entryPoint(entryPoint)
{
}

ShaderMask getShaderMask(ShaderType type)
{
	if (static_cast<uint8_t>(type) > EnumCount<ShaderType>())
		return ShaderMask::None;
	return static_cast<ShaderMask>(1 << EnumToIndex(type));
}

ShaderHandle Shader::create(const char* name, ShaderType type, const char* entryPoint, const void* content, size_t size)
{
	return Application::app()->graphic()->createShader(name, type, entryPoint, content, size);
}

void Shader::destroy(ShaderHandle shader)
{
	Application::app()->graphic()->destroy(shader);
}

};
};