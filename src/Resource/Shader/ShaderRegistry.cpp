#include <Aka/Resource/Shader/ShaderRegistry.h>

#include <Aka/Resource/Shader/ShaderCompiler.h>
#include <Aka/Resource/Shader/Shader.h>

namespace aka {

ShaderType getShaderMask(const ProgramKey& key)
{
	ShaderType type = ShaderType::None;
	for (const auto& elem : key.shaders)
	{
		AKA_ASSERT((type & elem.type) == ShaderType::None, "Shader type already in program key. Invalid");
		type |= elem.type;
	}
	return type;
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
				bindings.bindings[i].shaderType = bindings.bindings[i].shaderType | rhs.bindings[i].shaderType;
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
	// TODO delete all shaders to avoid leak
}

void ShaderRegistry::add(const ProgramKey& key, gfx::GraphicDevice* device)
{
	ShaderCompiler compiler; // TODO opti cache this in registry
	ShaderType mask = getShaderMask(key);
	const bool hasVertexStage = has(mask, ShaderType::Vertex);
	const bool hasFragmentStage = has(mask, ShaderType::Fragment);
	const bool hasGeometryStage = has(mask, ShaderType::Geometry);
	const bool hasComputeStage = has(mask, ShaderType::Compute);

	const bool isVertexProgram = hasVertexStage && hasFragmentStage;
	const bool isVertexGeometryProgram = isVertexProgram && hasGeometryStage;
	const bool isComputeProgram = hasComputeStage;
	AKA_ASSERT(!(isVertexProgram && isComputeProgram) && (isVertexProgram || isComputeProgram), "Unknown stage");

	const size_t ShaderTypeCount = 6;
	Array<ShaderBlob, ShaderTypeCount> blobs;
	Array<ShaderData, ShaderTypeCount> datas;
	for (auto& shader : key.shaders)
	{
		uint32_t index = EnumToIntegral(shader.type);
		// TODO opti if same macro and path, merge compilation
		blobs[index] = compiler.compile(shader);
		datas[index] = compiler.reflect(blobs[index], shader.entryPoint.cstr());
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
	gfx::ProgramHandle program;
	if (isVertexProgram)
	{
		ShaderBlob& vertexBlob = blobs[EnumToIntegral(gfx::ShaderType::Vertex)];
		ShaderBlob& fragBlob = blobs[EnumToIntegral(gfx::ShaderType::Fragment)];
		program = device->createProgram(
			device->createShader(gfx::ShaderType::Vertex, vertexBlob.data(), vertexBlob.size()),
			device->createShader(gfx::ShaderType::Fragment, fragBlob.data(), fragBlob.size()),
			gfx::ShaderHandle::null,
			states,
			static_cast<uint32_t>(setCount)
		);
	}
	else if (isVertexGeometryProgram)
	{
		ShaderBlob& vertexBlob = blobs[EnumToIntegral(gfx::ShaderType::Vertex)];
		ShaderBlob& fragBlob = blobs[EnumToIntegral(gfx::ShaderType::Fragment)];
		ShaderBlob& geoBlob = blobs[EnumToIntegral(gfx::ShaderType::Geometry)];
		program = device->createProgram(
			device->createShader(gfx::ShaderType::Vertex, vertexBlob.data(), vertexBlob.size()),
			device->createShader(gfx::ShaderType::Fragment, fragBlob.data(), fragBlob.size()),
			device->createShader(gfx::ShaderType::Geometry, geoBlob.data(), geoBlob.size()),
			states,
			static_cast<uint32_t>(setCount)
		);
	}
	else if (isComputeProgram)
	{
		ShaderBlob& computeBlob = blobs[EnumToIntegral(gfx::ShaderType::Compute)];
		AKA_NOT_IMPLEMENTED;
	}
	if (program != gfx::ProgramHandle::null)
		m_programs.insert(std::make_pair(key, program));
}

void ShaderRegistry::remove(const ProgramKey& key, gfx::GraphicDevice* device)
{
}

void ShaderRegistry::destroy(gfx::GraphicDevice* device)
{
}

gfx::ProgramHandle ShaderRegistry::get(const ProgramKey& key)
{
	auto it = m_programs.find(key);
	if (it == m_programs.end())
		return gfx::ProgramHandle::null;
	return it->second;
}

};