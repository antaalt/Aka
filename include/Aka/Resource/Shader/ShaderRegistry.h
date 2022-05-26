#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Resource/Shader/Shader.h>
#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/GraphicDevice.h>

#include <unordered_map>

namespace aka {

class ShaderRegistry
{
public:
	ShaderRegistry();
	~ShaderRegistry();

	// Add a program to the registry
	void add(const ProgramKey& key, gfx::GraphicDevice* device);
	// Remove a program from registry
	void remove(const ProgramKey& key, gfx::GraphicDevice* device);
	// Destroy all program from registry
	void destroy(gfx::GraphicDevice* device);

	// Get the program.
	gfx::ProgramHandle get(const ProgramKey& key);

	//on shader change, recompile all program that were using it
	void update() {} // TODO hot reload

	std::unordered_map<ProgramKey, gfx::ProgramHandle>::iterator begin() { return m_programs.begin(); }
	std::unordered_map<ProgramKey, gfx::ProgramHandle>::iterator end() { return m_programs.end(); }
	std::unordered_map<ProgramKey, gfx::ProgramHandle>::const_iterator begin() const { return m_programs.begin(); }
	std::unordered_map<ProgramKey, gfx::ProgramHandle>::const_iterator end() const { return m_programs.end(); }
private:
	std::unordered_map<ProgramKey, gfx::ProgramHandle> m_programs;
};

};