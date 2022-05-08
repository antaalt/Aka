#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Core/Application.h>

namespace aka {

struct ProgramReloadedEvent
{
	String name;
	gfx::ProgramHandle program;
};

class ProgramManager :
	EventListener<AppUpdateEvent>
{
public:
	ProgramManager() {}
	~ProgramManager();

	// Get a program from its name
	gfx::ProgramHandle get(const String& name);
	// Get a shader from its name
	gfx::ShaderHandle getShader(const String& name);
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
	gfx::ShaderHandle compile(const Path& path, gfx::ShaderType type, gfx::ShaderBindingState* bindings, uint32_t* setCount, gfx::VertexBindingState* vertices);
private:
	struct ShaderInfo {
		String name;
		gfx::ShaderType type;
		gfx::ShaderHandle shader;
		Path path;
		uint32_t setCount;
		gfx::ShaderBindingState sets[gfx::ShaderBindingState::MaxSetCount];
		gfx::VertexBindingState vertices;
		Timestamp loaded;
	};
	struct ProgramInfo {
		String name;
		gfx::ProgramHandle program;
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