#include <Aka/Resource/Shader/Shader.h>

namespace aka {

bool ShaderKey::operator<(const ShaderKey& lhs) const
{
	AKA_NOT_IMPLEMENTED;
	return false;
}
bool ShaderKey::operator==(const ShaderKey& lhs) const
{
	AKA_NOT_IMPLEMENTED;
	return false;
}

bool ProgramKey::operator<(const ProgramKey& lhs) const
{
	AKA_NOT_IMPLEMENTED;
	return false;
}
bool ProgramKey::operator==(const ProgramKey& lhs) const
{
	if (shaders.size() != lhs.shaders.size())
		return false;
	for (uint32_t i = 0; i < shaders.size(); i++)
	{
		if (shaders[i].entryPoint != lhs.shaders[i].entryPoint)
			return false;
		if (shaders[i].type != lhs.shaders[i].type)
			return false;
		if (shaders[i].macros != lhs.shaders[i].macros)
			return false;
		if (shaders[i].path != lhs.shaders[i].path)
			return false;
	}
	return true;
}

};