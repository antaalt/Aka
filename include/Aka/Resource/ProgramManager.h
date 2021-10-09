#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Core/Application.h>

namespace aka {

struct ProgramReloadedEvent
{
	String name;
	Program::Ptr program;
};

class ProgramManager :
	EventListener<AppUpdateEvent>
{
public:
	ProgramManager() {}
	~ProgramManager() {}

	// Get a program from its name
	Program::Ptr get(const String& name);
	// Get a shader from its name
	Shader::Ptr getShader(const String& name);
	// Reload a specific shader.
	bool reload(const String& name);

	// Parse a shader.json referencing all imported shaders
	bool parse(const Path& path);
	// Write a shader.json referencing all imported shaders
	bool serialize(const Path& path);

private:
	// Check the shaders and recompile them if they where updated
	void onReceive(const AppUpdateEvent& event);
private:
	// Compile a shader using GLSLCC for currently used graphic API
	Shader::Ptr compile(const Path& path, ShaderType type, const VertexAttribute* attributes, size_t count);
private:
	struct ShaderInfo {
		String name;
		ShaderType type;
		Shader::Ptr shader;
		Path path;
		std::vector<VertexAttribute> attributes;
		Timestamp loaded;
	};
	struct ProgramInfo {
		String name;
		Program::Ptr program;
		String vert;
		String frag;
		String comp;
	};

	ShaderInfo& getShaderInfo(const String& name)
	{
		for (ShaderInfo& info : m_shaders)
			if (info.name == name)
				return info;
		throw std::invalid_argument("No shader with this name.");
	}
	ProgramInfo& getProgramInfo(const String& name)
	{
		for (ProgramInfo& info : m_programs)
			if (info.name == name)
				return info;
		throw std::invalid_argument("No program with this name.");
	}
	std::vector<ShaderInfo> m_shaders;
	std::vector<ProgramInfo> m_programs;
};

};