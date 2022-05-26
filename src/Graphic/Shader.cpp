#include <Aka/Graphic/Shader.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

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