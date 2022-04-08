#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Core/Application.h>

namespace aka {

struct ProgramReloadedEvent
{
	String name;
	Program* program;
};

class ProgramManager :
	EventListener<AppUpdateEvent>
{
public:
	ProgramManager() {}
	~ProgramManager();

	// Get a program from its name
	Program* get(const String& name);
	// Get a shader from its name
	Shader* getShader(const String& name);
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
	Shader* compile(const Path& path, ShaderType type, ShaderBindingState* bindings, uint32_t* setCount, VertexBindingState* vertices);
private:
	struct ShaderInfo {
		String name;
		ShaderType type;
		Shader* shader;
		Path path;
		uint32_t setCount;
		ShaderBindingState sets[ShaderBindingState::MaxSetCount];
		VertexBindingState vertices;
		Timestamp loaded;
	};
	struct ProgramInfo {
		String name;
		Program* program;
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