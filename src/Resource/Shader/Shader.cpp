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

const char* toString(gfx::ShaderType type)
{
	switch (type)
	{
	case ShaderType::Vertex: return "Vertex";
	case ShaderType::Fragment: return "Fragment";
	case ShaderType::Compute: return "Compute";
	case ShaderType::Task: return "Task";
	case ShaderType::Mesh: return "Mesh";
	case ShaderType::TessControl: return "TessControl";
	case ShaderType::TessEvaluation: return "TessEvaluation";
	default:
	case ShaderType::Unknown: return "Unknown";
	}
}

std::ostream& operator<<(std::ostream& os, const ShaderKey& key)
{
	aka::String macros;
	for (aka::String macro : key.macros)
	{
		macros.append(macro);
		if (macro != key.macros.last())
			macros.append(",");
	}
	if (key.macros.size() == 0)
		macros.append("ø");
	return os << "ShaderKey(" << key.path << "," << key.entryPoint << "," << macros << "," << toString(key.type) << ")";
}

};