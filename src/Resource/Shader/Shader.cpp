#include <Aka/Resource/Shader/Shader.h>
#include <Aka/OS/OS.h>

namespace aka {

ShaderKey ShaderKey::generate(const AssetPath& path, ShaderType type)
{
	return ShaderKey{
		path,
		Vector<String>(),
		type,
		"main"
	};
}

bool operator<(const ShaderKey& lhs, const ShaderKey& rhs)
{
	AKA_NOT_IMPLEMENTED;
	return false;
}
bool operator==(const ShaderKey& lhs, const ShaderKey& rhs)
{
	return (rhs.entryPoint == lhs.entryPoint) && (rhs.macros == lhs.macros) && (rhs.type == lhs.type) && (rhs.path == lhs.path);
}

bool operator<(const ProgramKey& lhs, const ProgramKey& rhs)
{
	AKA_NOT_IMPLEMENTED;
	return false;
}
bool operator==(const ProgramKey& lhs, const ProgramKey& rhs)
{
	if (rhs.shaders.size() != lhs.shaders.size())
		return false;
	for (uint32_t i = 0; i < rhs.shaders.size(); i++)
	{
		if (rhs.shaders[i].entryPoint != lhs.shaders[i].entryPoint)
			return false;
		if (rhs.shaders[i].type != lhs.shaders[i].type)
			return false;
		if (rhs.shaders[i].macros != lhs.shaders[i].macros)
			return false;
		if (rhs.shaders[i].path != lhs.shaders[i].path)
			return false;
	}
	return true;
}

};