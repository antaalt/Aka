#include <Aka/Graphic/Shader.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

bool has(ShaderType flags, ShaderType flag)
{
	return (flags & flag) == flag;
}

ShaderType operator&(ShaderType lhs, ShaderType rhs)
{
	return static_cast<ShaderType>(
		static_cast<std::underlying_type<ShaderType>::type>(lhs) &
		static_cast<std::underlying_type<ShaderType>::type>(rhs)
	);
}
ShaderType operator|(ShaderType lhs, ShaderType rhs)
{
	return static_cast<ShaderType>(
		static_cast<std::underlying_type<ShaderType>::type>(lhs) |
		static_cast<std::underlying_type<ShaderType>::type>(rhs)
	);
}

ShaderHandle Shader::compile(ShaderType type, const uint8_t* content, size_t size)
{
	return Application::app()->graphic()->compile(type, content, size);
}

void Shader::destroy(ShaderHandle shader)
{
	Application::app()->graphic()->destroy(shader);
}

};
};