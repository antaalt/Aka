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

gfx::ShaderConstant merge(const gfx::ShaderConstant& lhs, const gfx::ShaderConstant& rhs)
{
	gfx::ShaderConstant constant = lhs;
	if (lhs.shader != gfx::ShaderMask::None || rhs.shader != gfx::ShaderMask::None)
	{
		if (lhs.shader != gfx::ShaderMask::None && rhs.shader != gfx::ShaderMask::None)
		{
			AKA_ASSERT(rhs.size == lhs.size, "Mismatching size");
			AKA_ASSERT(rhs.offset == lhs.offset, "Mismatching offset");
		}
		constant.shader = lhs.shader | rhs.shader;
		return constant;
	}
	return constant;
}

ShaderRegistry::ShaderRegistry()
{
}

ShaderRegistry::~ShaderRegistry()
{
	AKA_ASSERT(m_programs.size() == 0, "Missing programs");
	AKA_ASSERT(m_shaders.size() == 0, "Missing shaders");
}

void ShaderRegistry::add(const ProgramKey& key, gfx::GraphicDevice* device)
{
	ShaderCompiler compiler; // TODO opti cache this in registry
	gfx::ShaderMask mask = getShaderMask(key);
	const bool hasVertexStage = has(mask, gfx::ShaderMask::Vertex);
	const bool hasFragmentStage = has(mask, gfx::ShaderMask::Fragment);
	const bool hasTaskStage = has(mask, gfx::ShaderMask::Task);
	const bool hasMeshStage = has(mask, gfx::ShaderMask::Mesh);
	const bool hasComputeStage = has(mask, gfx::ShaderMask::Compute);
	// TODO: add tesselation + ray support

	const bool isVertexProgram = hasVertexStage && hasFragmentStage;
	const bool isMeshProgram = hasMeshStage && hasFragmentStage; // task shader optional
	const bool isComputeProgram = hasComputeStage;
	AKA_ASSERT(!(isVertexProgram && isComputeProgram) && !(isVertexProgram && isMeshProgram) && (isVertexProgram || isComputeProgram || isMeshProgram), "Unknown stage");

	constexpr size_t ShaderTypeCount = EnumCount<ShaderType>();
	Array<ShaderData, ShaderTypeCount> datas;
	Array<gfx::ShaderHandle, ShaderTypeCount> shaders;
	bool shaderReplaced = false;
	for (auto& shaderKey : key.shaders)
	{
		uint32_t index = EnumToIndex(shaderKey.type);
		
		// TODO: do not always recompute shader if it already exist. Share it between programs.
		// TODO: opti if same macro and path, merge compilation for multiple elements
		// TODO: use multiple entry points
		// TODO: cache blob for same file.
		Blob blob = compiler.compile(shaderKey);
#if defined(AKA_DEBUG)
		while (blob.size() == 0)
		{
			String errorMessage = "Failed to compile shader :";
			errorMessage += shaderKey.path.cstr();
			AlertModalMessage result = AlertModal(AlertModalType::Warning, "Failed shader compilation", errorMessage.cstr());
			blob = compiler.compile(shaderKey);
		} 
#endif
		AKA_ASSERT(blob.size() > 0, "Failed to compile shader");
		datas[index] = compiler.reflect(blob, shaderKey.entryPoint.cstr());
		shaders[index] = device->createShader(OS::File::basename(shaderKey.path).cstr(), shaderKey.type, blob.data(), blob.size());
		auto itShader = m_shaders.insert(std::make_pair(shaderKey, shaders[index]));
		if (!itShader.second)
		{
			// Need to update pipelines
			shaderReplaced = true;
			// Destroy old one
			device->destroy(itShader.first->second);
			// Replace register
			m_shaders[shaderKey] = shaders[index];
			m_shadersFileData[shaderKey] = ShaderFileData{ Timestamp::now() };
		}
		else
		{
			auto itShaderData = m_shadersFileData.insert(std::make_pair(shaderKey, ShaderFileData{ Timestamp::now() }));
			AKA_ASSERT(itShaderData.second, "Failed to insert shader data");
		}
	}
	// Merge shader bindings
	gfx::ShaderPipelineLayout layout;
	for (const ShaderData& data : datas) // for each shader
	{
		layout.setCount = max((uint32_t)data.sets.size(), layout.setCount);
		for (uint32_t iSet = 0; iSet < data.sets.size(); iSet++)
		{
			layout.sets[iSet] = merge(data.sets[iSet], layout.sets[iSet]);
		}
	}
	// Merge shader constants
	for (const ShaderData& data : datas) // for each shader
	{
		layout.constantCount = max((uint32_t)data.constants.size(), layout.constantCount);
		for (uint32_t iCst = 0; iCst < data.constants.size(); iCst++)
		{
			layout.constants[iCst] = merge(data.constants[iCst], layout.constants[iCst]);
		}
	}
	// Create shaders
	String name;
	for (auto shaders : key.shaders)
	{
		name += OS::File::name(shaders.path);
	}
	gfx::ProgramHandle program;
	if (isVertexProgram)
	{
		name = "GraphicProgram" + name;
		program = device->createVertexProgram(
			name.cstr(),
			shaders[EnumToIndex(gfx::ShaderType::Vertex)],
			shaders[EnumToIndex(gfx::ShaderType::Fragment)],
			layout
		);
	}
	else if (isMeshProgram)
	{
		name = "GraphicMeshProgram" + name;
		program = device->createMeshProgram(
			name.cstr(),
			hasTaskStage ? shaders[EnumToIndex(gfx::ShaderType::Task)] : gfx::ShaderHandle::null,
			shaders[EnumToIndex(gfx::ShaderType::Mesh)],
			shaders[EnumToIndex(gfx::ShaderType::Fragment)],
			layout
		);
	}
	else if (isComputeProgram)
	{
		name = "ComputeProgram" + name;
		program = device->createComputeProgram(
			name.cstr(),
			shaders[EnumToIndex(gfx::ShaderType::Compute)],
			layout
		);
	}
	if (program != gfx::ProgramHandle::null)
	{
		auto it = m_programs.insert(std::make_pair(key, program));
		if (!it.second || shaderReplaced)
		{
			// Already exist, we need to replace it.
			device->replace(it.first->second, program);
			// Destroy old one
			device->destroy(it.first->second);
			// Replace register
			m_programs[key] = program;
		}
	}
}

void ShaderRegistry::remove(const ProgramKey& key, gfx::GraphicDevice* device)
{
	auto getRefCount = [&](const ShaderKey& key) -> uint32_t {
		uint32_t refCount = 0;
		for (auto& program : m_programs)
		{
			for (auto& shader : program.first.shaders)
			{
				if (shader == key)
				{
					refCount++;
				}
			}
		}
		return refCount;
	};
	auto itProgram = m_programs.find(key);
	if (itProgram != m_programs.end())
	{
		ProgramKey programKey = itProgram->first;
		gfx::ProgramHandle programHandle = itProgram->second;
		const gfx::Program* program = device->get(programHandle);

		for (gfx::ShaderType shaderType : EnumRange<gfx::ShaderType>())
		{
			if (program->shaders[EnumToIndex(shaderType)] != gfx::ShaderHandle::null)
			{
				for (auto shaderKey : programKey.shaders)
				{
					if (shaderKey.type == shaderType)
					{
						device->destroy(program->shaders[EnumToIndex(shaderType)]);
						auto itShader = m_shaders.find(shaderKey);
						if (itShader != m_shaders.end())
							m_shaders.erase(itShader);
						auto itShaderData = m_shadersFileData.find(shaderKey);
						if (itShaderData != m_shadersFileData.end())
							m_shadersFileData.erase(itShaderData);
						break; // key should not be duplicated
					}
				}
			}
		}
		device->destroy(programHandle);
		m_programs.erase(itProgram);
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

void ShaderRegistry::reload(const ShaderKey& _shaderKey, gfx::GraphicDevice* device)
{
	ShaderKey shaderKey = _shaderKey; // Store shaderKey before it gets invalidated
	// Check if shader compile before removing it and reloading it.
	ShaderCompiler compiler;
	Blob blob = compiler.compile(shaderKey);
	if (blob.size() == 0)
	{
		Logger::error("Failed to compile shader at : ", shaderKey.path);
		return;
	}
	// Get all dependent program to recompile.
	std::vector<ProgramKey> programToReload;
	for (auto& program : m_programs)
	{
		for (auto& shader : program.first.shaders)
		{
			if (shader.path == shaderKey.path)
			{
				programToReload.push_back(program.first);
			}
		}
	}
	// Send events.
	for (auto program : programToReload)
	{
		auto itProgram = m_programs.find(program);
		if (itProgram != m_programs.end())
		{
			gfx::ProgramHandle programHandle = itProgram->second;

			// Compile new program. 
			// This should take care of updating dependencies aswell
			add(program, device);
		}
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
			ShaderKey key = shader.first;
			//Date date = Date::globaltime(m_shadersFileData[key].timestamp);
			Logger::info("Shader reloaded : ", shader.first.path);
			reload(key, device);
			break; // m_shaders is invalidated
		}
	}
}

};