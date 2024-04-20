#include <Aka/Resource/Shader/ShaderRegistry.h>

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Shader/ShaderCompiler.h>
#include <Aka/Resource/Shader/Shader.h>
#include <Aka/Core/Application.h>
#include <Aka/OS/OS.h>
#include <Aka/OS/StopWatch.h>

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

using ShaderDataArray = Array<ShaderReflectionData, EnumCount<ShaderType>()>;
using ShaderHandleArray = Array<gfx::ShaderHandle, EnumCount<ShaderType>()>;

std::tuple<gfx::ShaderHandle, ShaderReflectionData> CompileShader(const ShaderKey& _shaderKey, gfx::GraphicDevice* _device, ShaderCompiler& _compiler)
{
	StopWatch stopWatch;
	ShaderBlob shaderBlob = _compiler.compile(_shaderKey);
	Logger::info("Shader ", _shaderKey.path, " compiled in ", stopWatch.elapsed(), "ms");
#if defined(AKA_SHADER_HOT_RELOAD)
	while (shaderBlob.size() == 0)
	{
		// TODO retrieve errors here & handle them here instead ?
		String errorMessage = String::format("Failed to compile shader %s\n Retry ?", _shaderKey.path.cstr());
		AlertModalMessage result = AlertModal(AlertModalType::Question, "Failed shader compilation", errorMessage.cstr());
		if (result == AlertModalMessage::No)
		{
			return std::make_tuple(gfx::ShaderHandle::null, ShaderReflectionData{});
		}
		stopWatch.start();
		shaderBlob = _compiler.compile(_shaderKey);
		Logger::info("Shader ", _shaderKey.path, " compiled in ", stopWatch.elapsed(), "ms");
	}
#else
	if (shaderBlob.size() == 0)
	{
		return std::make_tuple(gfx::ShaderHandle::null, ShaderReflectionData{});
	}
#endif
	String shaderDebugName = OS::File::name(_shaderKey.path.getAbsolutePath());
	ShaderReflectionData data = _compiler.reflect(shaderBlob, _shaderKey.entryPoint.cstr());
	gfx::ShaderHandle handle = _device->createShader(shaderDebugName.cstr(), _shaderKey.type, shaderBlob.data(), shaderBlob.size());
	return std::make_tuple(handle, data);
}

gfx::ProgramHandle CompileProgram(const ProgramKey& _programKey, gfx::GraphicDevice* _device, const ShaderDataArray& datas, const ShaderHandleArray& shaders)
{
	gfx::ShaderMask mask = getShaderMask(_programKey);
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

	// Merge shader bindings
	gfx::ShaderPipelineLayout layout;
	for (const ShaderReflectionData& data : datas) // for each shader
	{
		layout.setCount = max((uint32_t)data.sets.size(), layout.setCount);
		for (uint32_t iSet = 0; iSet < data.sets.size(); iSet++)
		{
			layout.sets[iSet] = merge(data.sets[iSet], layout.sets[iSet]);
		}
	}
	// Merge shader constants
	for (const ShaderReflectionData& data : datas) // for each shader
	{
		layout.constantCount = max((uint32_t)data.constants.size(), layout.constantCount);
		for (uint32_t iCst = 0; iCst < data.constants.size(); iCst++)
		{
			layout.constants[iCst] = merge(data.constants[iCst], layout.constants[iCst]);
		}
	}
	// Create shaders
	String name;
	for (const ShaderKey& shaders : _programKey.shaders)
	{
		name += OS::File::name(shaders.path.getAbsolutePath());
	}
	gfx::ProgramHandle program;
	if (isVertexProgram)
	{
		name = "GraphicProgram" + name;
		program = _device->createVertexProgram(
			name.cstr(),
			shaders[EnumToIndex(gfx::ShaderType::Vertex)],
			shaders[EnumToIndex(gfx::ShaderType::Fragment)],
			layout
		);
	}
	else if (isMeshProgram)
	{
		name = "GraphicMeshProgram" + name;
		program = _device->createMeshProgram(
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
		program = _device->createComputeProgram(
			name.cstr(),
			shaders[EnumToIndex(gfx::ShaderType::Compute)],
			layout
		);
	}
	return program;
}

bool ShaderRegistry::add(const ProgramKey& key, gfx::GraphicDevice* device)
{
	auto it = m_programs.find(key);
	if (it != m_programs.end())
	{
		// Program already registered.
		return false;
	}

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

	ShaderDataArray datas;
	ShaderHandleArray shaders;
	for (const ShaderKey& shaderKey : key.shaders)
	{
		uint32_t index = EnumToIndex(shaderKey.type);
		auto it = m_shaders.find(shaderKey);
		if (it != m_shaders.end())
		{
			// Shader already exist, pick it directly.
			auto itShaderData = m_shadersFileData.find(shaderKey);
			AKA_ASSERT(itShaderData != m_shadersFileData.end(), "Failed to find corresponding shader data");
			datas[index] = itShaderData->second.data;
			shaders[index] = it->second;
		}
		else
		{
			// Shader does not exist. Compile it & add it.
			std::tie(shaders[index], datas[index]) = CompileShader(shaderKey, device, m_compiler);
			// Check compilation result
			if (shaders[index] == gfx::ShaderHandle::null)
				return false;
			// Add to DB
			auto itShader = m_shaders.insert(std::make_pair(shaderKey, shaders[index]));
			AKA_ASSERT(itShader.second, "Failed to add shader to DB");
			auto itShaderData = m_shadersFileData.insert(std::make_pair(shaderKey, ShaderFileData{ datas[index], Timestamp::now() }));
			AKA_ASSERT(itShaderData.second, "Failed to add shader data");
		}
	}
	gfx::ProgramHandle program = CompileProgram(key, device, datas, shaders);
	if (program != gfx::ProgramHandle::null)
	{
		auto it = m_programs.insert(std::make_pair(key, program));
		AKA_ASSERT(it.second, "Failed to add program");
		return true;
	}
	else
	{
		return false;
	}
}

bool ShaderRegistry::remove(const ProgramKey& key, gfx::GraphicDevice* device)
{
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
		return true;
	}
	else
	{
		return false;
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

#if defined(AKA_SHADER_HOT_RELOAD)
bool ShaderRegistry::reload(const ShaderKey& _shaderKey, gfx::GraphicDevice* device)
{
	if (m_shaders.find(_shaderKey) == m_shaders.end())
	{
		// Shader does not exist in db, cant reload it.
		return false;
	}
	// Recompile shader
	ShaderCompiler compiler;
	std::tuple<gfx::ShaderHandle, ShaderReflectionData> shaderCompilation = CompileShader(_shaderKey, device, compiler);
	gfx::ShaderHandle shaderHandle = std::get<gfx::ShaderHandle>(shaderCompilation);
	ShaderReflectionData shaderData = std::get<ShaderReflectionData>(shaderCompilation);
	// Check compilation result
	if (shaderHandle == gfx::ShaderHandle::null)
		return false;

	// Destroy old shader
	device->destroy(m_shaders[_shaderKey]);
	// Replace register
	m_shaders[_shaderKey] = shaderHandle;
	m_shadersFileData[_shaderKey] = ShaderFileData{ shaderData, Timestamp::now() };

	// Get all dependant program to recompile.
	std::vector<ProgramKey> programToReload;
	for (std::pair<const ProgramKey, gfx::ProgramHandle>& program : m_programs)
	{
		for (const ShaderKey& programShader : program.first.shaders)
		{
			if (programShader.path == _shaderKey.path)
			{
				programToReload.push_back(program.first);
			}
		}
	}
	// Send events.
	for (const ProgramKey& programKey : programToReload)
	{
		AKA_ASSERT(m_programs.find(programKey) != m_programs.end(), "Failure in the matrix");

		ShaderDataArray datas;
		ShaderHandleArray shaders;
		for (const ShaderKey& shaderKey : programKey.shaders)
		{
			uint32_t index = EnumToIndex(shaderKey.type);
			if (_shaderKey == shaderKey)
			{
				shaders[index] = shaderHandle;
				datas[index] = shaderData;
			}
			else
			{
				auto it = m_shaders.find(shaderKey);
				auto itShaderData = m_shadersFileData.find(shaderKey);
				AKA_ASSERT(it != m_shaders.end(), "Failed to find corresponding shader");
				AKA_ASSERT(itShaderData != m_shadersFileData.end(), "Failed to find corresponding shader data");
				shaders[index] = it->second;
				datas[index] = itShaderData->second.data;
			}
		}

		gfx::ProgramHandle oldProgram = m_programs[programKey];
		gfx::ProgramHandle newProgram = CompileProgram(programKey, device, datas, shaders);

		// Already exist, we need to replace it.
		device->replace(oldProgram, newProgram);
		// Destroy old one
		device->destroy(oldProgram);
		// Replace register
		m_programs[programKey] = newProgram;
	}
	return true;
}

void ShaderRegistry::reloadIfChanged(gfx::GraphicDevice* device)
{
	// TODO: use a file watcher instead.
	// Here shaders might be recompiled to many time...
	for (auto& shader : m_shaders)
	{
		auto it = m_shadersFileData.find(shader.first);
		AKA_ASSERT(it != m_shadersFileData.end(), "");
		Timestamp updateDelay = Timestamp::seconds(1);
		bool updated = OS::File::lastWrite(shader.first.path.getAbsolutePath()) > it->second.timestamp;
		if (updated)
		{
			StopWatch stopWatch;
			Logger::info("Shader ", shader.first.path, " reloading...");
			reload(shader.first, device);
			Logger::info("Shader ", shader.first.path, " reloaded in : ", stopWatch.elapsed(), "ms");
			break; // m_shaders is invalidated
		}
	}
}
#endif

};