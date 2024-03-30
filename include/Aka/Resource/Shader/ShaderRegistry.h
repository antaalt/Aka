#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Resource/Shader/Shader.h>
#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/GraphicDevice.h>

#include <unordered_map>

namespace aka {

struct ShaderFileData
{
	Timestamp timestamp;
};

class ShaderRegistry
{
public:
	ShaderRegistry();
	ShaderRegistry(const ShaderRegistry&) = delete;
	ShaderRegistry(ShaderRegistry&&) = delete;
	ShaderRegistry& operator=(const ShaderRegistry&) = delete;
	ShaderRegistry& operator=(ShaderRegistry&&) = delete;
	~ShaderRegistry();

	// Add a program to the registry
	void add(const ProgramKey& key, gfx::GraphicDevice* device);
	// Remove a program from registry
	void remove(const ProgramKey& key, gfx::GraphicDevice* device);
	// Destroy all program from registry
	void destroy(gfx::GraphicDevice* device);

	// Get the program.
	gfx::ProgramHandle get(const ProgramKey& key) const;
	// Get the shader.
	gfx::ShaderHandle getShader(const ShaderKey& key) const;

	// Reload a specific shader
	void reload(const ShaderKey& key, gfx::GraphicDevice* device);
	// Reload all shaders that changed
	void reloadIfChanged(gfx::GraphicDevice* device);

	std::unordered_map<ProgramKey, gfx::ProgramHandle>::iterator begin() { return m_programs.begin(); }
	std::unordered_map<ProgramKey, gfx::ProgramHandle>::iterator end() { return m_programs.end(); }
	std::unordered_map<ProgramKey, gfx::ProgramHandle>::const_iterator begin() const { return m_programs.begin(); }
	std::unordered_map<ProgramKey, gfx::ProgramHandle>::const_iterator end() const { return m_programs.end(); }
private:
	std::unordered_map<ShaderKey, ShaderFileData> m_shadersFileData;
	std::unordered_map<ShaderKey, gfx::ShaderHandle> m_shaders;
	std::unordered_map<ProgramKey, gfx::ProgramHandle> m_programs;
};

};