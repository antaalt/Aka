#include <Aka/Graphic/Shader.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

ShaderHandle Shader::compile(ShaderType type, const void* content, size_t size)
{
	return Application::app()->graphic()->compile(type, content, size);
}

void Shader::destroy(ShaderHandle shader)
{
	Application::app()->graphic()->destroy(shader);
}

};
};