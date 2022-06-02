#include <Aka/Resource/Shader/ShaderRegistry.h>

#include <Aka/Resource/Shader/ShaderCompiler.h>
#include <Aka/Resource/Shader/Shader.h>
#include <Aka/Core/Application.h>
#include <Aka/OS/OS.h>

namespace aka {

gfx::ShaderMask getShaderMask(const ProgramKey& key)
{
	gfx::ShaderMask mask = gfx::ShaderMask::None;
	for (const auto& elem : key.shaders)
	{
		gfx::ShaderMask m = gfx::getShaderMask(elem.type);
		AKA_ASSERT((mask & m) == gfx::ShaderMask::None, "Shader type already in program key. Invalid");
		mask |= m;
	}
	return mask;
}

gfx::ShaderBindingState merge(const gfx::ShaderBindingState& lhs, const gfx::ShaderBindingState& rhs)
{
	gfx::ShaderBindingState bindings = lhs;
	for (uint32_t i = 0; i < rhs.count; i++)
	{
		if (rhs.bindings[i].type != gfx::ShaderBindingType::None)
		{
			if (lhs.bindings[i].type == gfx::ShaderBindingType::None)
			{
				bindings.bindings[i] = rhs.bindings[i];
				bindings.count = max(bindings.count, i + 1);
			}
			else
			{
				AKA_ASSERT(rhs.bindings[i].type == lhs.bindings[i].type, "Mismatching bindings");
				AKA_ASSERT(rhs.bindings[i].count == lhs.bindings[i].count, "Mismatching count");
				bindings.bindings[i].stages = bindings.bindings[i].stages | rhs.bindings[i].stages;
			}
		}
	}
	return bindings;
}

ShaderRegistry::ShaderRegistry()
{
}

ShaderRegistry::~ShaderRegistry()
{
	destroy(Application::app()->graphic());
}

void ShaderRegistry::add(const ProgramKey& key, gfx::GraphicDevice* device)
{
	ShaderCompiler compiler; // TODO opti cache this in registry
	gfx::ShaderMask mask = getShaderMask(key);
	const bool hasVertexStage = has(mask, gfx::ShaderMask::Vertex);
	const bool hasFragmentStage = has(mask, gfx::ShaderMask::Fragment);
	const bool hasGeometryStage = has(mask, gfx::ShaderMask::Geometry);
	const bool hasComputeStage = has(mask, gfx::ShaderMask::Compute);

	const bool isVertexProgram = hasVertexStage && hasFragmentStage;
	const bool isVertexGeometryProgram = isVertexProgram && hasGeometryStage;
	const bool isComputeProgram = hasComputeStage;
	AKA_ASSERT(!(isVertexProgram && isComputeProgram) && (isVertexProgram || isComputeProgram), "Unknown stage");

	const size_t ShaderTypeCount = 6;
	Array<ShaderData, ShaderTypeCount> datas;
	Array<gfx::ShaderHandle, ShaderTypeCount> shaders;
	for (auto& shaderKey : key.shaders)
	{
		uint32_t index = EnumToIntegral(shaderKey.type);
		auto it = m_shaders.find(shaderKey);
		if (it != m_shaders.end())
		{
			shaders[index] = it->second;
		}
		else
		{
			// TODO opti if same macro and path, merge compilation for multiple elements
			// TODO cache blob for same file.
			Blob blob = compiler.compile(shaderKey);
			datas[index] = compiler.reflect(blob, shaderKey.entryPoint.cstr());
			shaders[index] = device->createShader(shaderKey.type, blob.data(), blob.size());
			m_shaders.insert(std::make_pair(shaderKey, shaders[index]));
			m_shadersFileData.insert(std::make_pair(shaderKey, ShaderFileData{Timestamp::now()}));
		}
	}
	// Merge shader bindings
	size_t setCount = 0;
	gfx::ShaderBindingState states[gfx::ShaderMaxSetCount]{};
	for (const ShaderData& data : datas) // for each shader
	{
		setCount = max(data.sets.size(), setCount);
		for (uint32_t iSet = 0; iSet < data.sets.size(); iSet++)
		{
			states[iSet] = merge(data.sets[iSet], states[iSet]);
		}
	}
	// Create shaders
	gfx::ProgramHandle program;
	if (isVertexProgram)
	{
		program = device->createProgram(
			shaders[EnumToIntegral(gfx::ShaderType::Vertex)],
			shaders[EnumToIntegral(gfx::ShaderType::Fragment)],
			gfx::ShaderHandle::null,
			states,
			static_cast<uint32_t>(setCount)
		);
	}
	else if (isVertexGeometryProgram)
	{
		program = device->createProgram(
			shaders[EnumToIntegral(gfx::ShaderType::Vertex)],
			shaders[EnumToIntegral(gfx::ShaderType::Fragment)],
			shaders[EnumToIntegral(gfx::ShaderType::Geometry)],
			states,
			static_cast<uint32_t>(setCount)
		);
	}
	else if (isComputeProgram)
	{
		AKA_NOT_IMPLEMENTED;
	}
	if (program != gfx::ProgramHandle::null)
		m_programs.insert(std::make_pair(key, program));
}

void ShaderRegistry::remove(const ProgramKey& key, gfx::GraphicDevice* device)
{
	auto it = m_programs.find(key);
	if (it != m_programs.end())
	{
		device->destroy(it->second);
		m_programs.erase(it);
		// TODO delete shaders that are using this program ?
	}
}

void ShaderRegistry::destroy(gfx::GraphicDevice* device)
{
	for (auto& program : m_programs)
	{
		device->destroy(program.second);
	}
	m_programs.clear();
	for (auto& shader : m_shaders)
	{
		device->destroy(shader.second);
	}
	m_shaders.clear();
	m_shadersFileData.clear();
}

gfx::ProgramHandle ShaderRegistry::get(const ProgramKey& key) const
{
	auto it = m_programs.find(key);
	if (it == m_programs.end())
		return gfx::ProgramHandle::null;
	return it->second;
}

gfx::ShaderHandle ShaderRegistry::getShader(const ShaderKey& key) const
{
	auto it = m_shaders.find(key);
	if (it == m_shaders.end())
		return gfx::ShaderHandle::null;
	return it->second;
}

void ShaderRegistry::reload(const ShaderKey& shaderKey, gfx::GraphicDevice* device)
{
	auto it = m_shaders.find(shaderKey);
	if (it == m_shaders.end())
	{
		// Shader does not exist.
		return;
	}
	else
	{
		// Rebuild shader
		ShaderCompiler compiler;


		Blob blob = compiler.compile(shaderKey);
		ShaderData data = compiler.reflect(blob, shaderKey.entryPoint.cstr());
		gfx::ShaderHandle shader = device->createShader(shaderKey.type, blob.data(), blob.size());
		// Destroy old shader.
		device->destroy(it->second);
		m_shaders[shaderKey] = shader;
		m_shadersFileData[shaderKey].timestamp = Timestamp::now();
	}
	// Send events.
	for (auto& program : m_programs)
	{
		for (auto& shader : program.first.shaders)
		{
			if (shader == shaderKey)
			{
				EventDispatcher<ShaderReloadedEvent>::emit(ShaderReloadedEvent{ shader, program.first });
			}
		}
		EventDispatcher<ShaderReloadedEvent>::dispatch();
	}
}

void ShaderRegistry::reloadIfChanged(gfx::GraphicDevice* device)
{
	for (auto& shader : m_shaders)
	{
		auto it = m_shadersFileData.find(shader.first);
		AKA_ASSERT(it != m_shadersFileData.end(), "");
		bool updated = OS::File::lastWrite(shader.first.path) > it->second.timestamp;
		if (updated)
		{
			reload(shader.first, device);
		}
	}
}

};