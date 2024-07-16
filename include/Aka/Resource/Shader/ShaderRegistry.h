#pragma once

#include <Aka/Core/Config.h>
#include <Aka/Resource/Shader/Shader.h>
#include <Aka/Graphic/Program.h>
#include <Aka/Resource/Shader/ShaderCompiler.h>
#include <Aka/OS/FileWatcher.hpp>

#include <unordered_map>
#include <mutex>

#define AKA_SHADER_HOT_RELOAD 1

namespace aka {

namespace gfx { class GraphicDevice; }

struct ShaderFileData
{
	ShaderReflectionData data;
	// These data are mostly used for hot reload.
	Timestamp timestamp;
	// TODO: could add dependencies...
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

	// Add a program to the registry. Return false if it already exist.
	bool add(const ProgramKey& key, gfx::GraphicDevice* device);
	// Remove a program from registry. Return false if it does not exist.
	bool remove(const ProgramKey& key, gfx::GraphicDevice* device);
	// Destroy all program from registry
	void destroy(gfx::GraphicDevice* device);

	// Get the program.
	gfx::ProgramHandle get(const ProgramKey& key) const;
	// Get the shader.
	gfx::ShaderHandle getShader(const ShaderKey& key) const;

#if defined(AKA_SHADER_HOT_RELOAD)
	// Reload a specific shader
	bool reload(const ShaderKey& _shaderKey, gfx::GraphicDevice* device);
	// Reload all shaders that changed
	void reloadIfChanged(gfx::GraphicDevice* device);
#endif

	std::unordered_map<ProgramKey, gfx::ProgramHandle>::iterator begin() { return m_programs.begin(); }
	std::unordered_map<ProgramKey, gfx::ProgramHandle>::iterator end() { return m_programs.end(); }
	std::unordered_map<ProgramKey, gfx::ProgramHandle>::const_iterator begin() const { return m_programs.begin(); }
	std::unordered_map<ProgramKey, gfx::ProgramHandle>::const_iterator end() const { return m_programs.end(); }
private:
	ShaderCompiler m_compiler;
	std::unordered_map<ShaderKey, ShaderFileData> m_shadersFileData;
	std::unordered_map<ShaderKey, gfx::ShaderHandle> m_shaders;
	std::unordered_map<ProgramKey, gfx::ProgramHandle> m_programs;
#if defined(AKA_SHADER_HOT_RELOAD)
	FileWatcher m_fileWatcher;
	std::mutex m_fileWatcherMutex;
	Vector<ShaderKey> m_shaderToReload;
	ShaderKey m_shaderReloading;
#endif
};

};