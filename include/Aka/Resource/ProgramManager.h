#pragma once

#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/ShaderMaterial.h>

namespace aka {

struct ProgramReloadedEvent
{
	String name;
	Shader::Ptr program;
};

// TODO rename ShaderMaterial Material
// TODO rename Shader -> Program & ShaderHandle -> Shader (and make it RAII)
class ProgramManager
{
public:
	// Get a program from its name
	static Shader::Ptr get(const String& name);
	// Get a shader from its name
	static ShaderHandle getShader(const String& name);
	// Reload a specific shader.
	static bool reload(const String& name);

	// Parse a shader.json referencing all imported shaders
	static bool parse(const Path& path);
	// Write a shader.json referencing all imported shaders
	static bool serialize(const Path& path);

	// Check the shaders and recompile them if they where updated
	// TODO use file watcher instead
	static void update();
private:
	// Compile a shader using GLSLCC for currently used graphic API
	static ShaderHandle compile(const Path& path, ShaderType type, const VertexAttribute* attributes, size_t count);
private:
	struct ShaderInfo {
		String name;
		ShaderType type;
		ShaderHandle shader;
		Path path;
		std::vector<VertexAttribute> attributes;
		Time::Unit loaded;
	};
	struct ProgramInfo {
		String name;
		Shader::Ptr program;
		String vert;
		String frag;
		String comp;
	};

	static ShaderInfo& getShaderInfo(const String& name)
	{
		for (ShaderInfo& info : m_shaders)
			if (info.name == name)
				return info;
		throw std::invalid_argument("No shader with this name.");
	}
	static ShaderInfo& getProgramInfo(const String& name)
	{
		for (ShaderInfo& info : m_shaders)
			if (info.name == name)
				return info;
		throw std::invalid_argument("No program with this name.");
	}
	static std::vector<ShaderInfo> m_shaders;
	static std::vector<ProgramInfo> m_programs;
};

};