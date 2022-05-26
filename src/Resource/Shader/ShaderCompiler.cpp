#include <Aka/Resource/Shader/ShaderCompiler.h>

#include <Aka/Resource/Shader/Shader.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/OS/Image.h>
#include <Aka/OS/OS.h>
#include <Aka/OS/Logger.h>

#include <SPIRV/GlslangToSpv.h>
#include <SPIRV/SpvTools.h>
#include <SPIRV/disassemble.h>
#include <SPIRV/spirv.hpp>

#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>

namespace aka {

const TBuiltInResource defaultConf = {
	32, // .MaxLights
	6, // .MaxClipPlanes
	32, // .MaxTextureUnits
	32, // .MaxTextureCoords
	64, // .MaxVertexAttribs
	4096, // .MaxVertexUniformComponents
	64, // .MaxVaryingFloats
	32, // .MaxVertexTextureImageUnits
	80, // .MaxCombinedTextureImageUnits
	32, // .MaxTextureImageUnits
	4096, // .MaxFragmentUniformComponents
	32, // .MaxDrawBuffers
	128, // .MaxVertexUniformVectors
	8, // .MaxVaryingVectors
	16, // .MaxFragmentUniformVectors
	16, // .MaxVertexOutputVectors
	15, // .MaxFragmentInputVectors
	-8, // .MinProgramTexelOffset,
	7, // .MaxProgramTexelOffset
	8, // .MaxClipDistances
	65535, // .MaxComputeWorkGroupCountX
	65535, // .MaxComputeWorkGroupCountY
	65535, // .MaxComputeWorkGroupCountZ
	1024, // .MaxComputeWorkGroupSizeX
	1024, // .MaxComputeWorkGroupSizeY
	64, // .MaxComputeWorkGroupSizeZ
	1024, // .MaxComputeUniformComponents
	16, // .MaxComputeTextureImageUnits
	8, // .MaxComputeImageUniforms
	8, // .MaxComputeAtomicCounters
	1, // .MaxComputeAtomicCounterBuffers
	60, // .MaxVaryingComponents
	64, // .MaxVertexOutputComponents
	64, // .MaxGeometryInputComponents
	128, // .MaxGeometryOutputComponents
	128, // .MaxFragmentInputComponents
	8, // .MaxImageUnits
	8, // .MaxCombinedImageUnitsAndFragmentOutputs
	8, // .MaxCombinedShaderOutputResources
	0, // .MaxImageSamples
	0, // .MaxVertexImageUniforms
	0, // .MaxTessControlImageUniforms
	0, // .MaxTessEvaluationImageUniforms
	0, // .MaxGeometryImageUniforms
	8, // .MaxFragmentImageUniforms
	8, // .MaxCombinedImageUniforms
	16, // .MaxGeometryTextureImageUnits
	256, // .MaxGeometryOutputVertices
	1024, // .MaxGeometryTotalOutputComponents
	1024, // .MaxGeometryUniformComponents
	64, // .MaxGeometryVaryingComponents
	128, // .MaxTessControlInputComponents
	128, // .MaxTessControlOutputComponents
	16, // .MaxTessControlTextureImageUnits
	1024, // .MaxTessControlUniformComponents
	4096, // .MaxTessControlTotalOutputComponents
	128, // .MaxTessEvaluationInputComponents
	128, // .MaxTessEvaluationOutputComponents
	16, // .MaxTessEvaluationTextureImageUnits
	1024, // .MaxTessEvaluationUniformComponents
	120, // .MaxTessPatchComponents
	32, // .MaxPatchVertices
	64, // .MaxTessGenLevel
	16, // .MaxViewports
	0, // .MaxVertexAtomicCounters
	0, // .MaxTessControlAtomicCounters
	0, // .MaxTessEvaluationAtomicCounters
	0, // .MaxGeometryAtomicCounters
	8, // .MaxFragmentAtomicCounters
	8, // .MaxCombinedAtomicCounters
	1, // .MaxAtomicCounterBindings
	0, // .MaxVertexAtomicCounterBuffers
	0, // .MaxTessControlAtomicCounterBuffers
	0, // .MaxTessEvaluationAtomicCounterBuffers
	0, // .MaxGeometryAtomicCounterBuffers
	1, // .MaxFragmentAtomicCounterBuffers
	1, // .MaxCombinedAtomicCounterBuffers
	16384, // .MaxAtomicCounterBufferSize
	4, // .MaxTransformFeedbackBuffers
	64, // .MaxTransformFeedbackInterleavedComponents
	8, // .MaxCullDistances
	8, // .MaxCombinedClipAndCullDistances
	4, // .MaxSamples
	256, // .maxMeshOutputVerticesNV
	512, // .maxMeshOutputPrimitivesNV
	32, // .maxMeshWorkGroupSizeX_NV
	1, // .maxMeshWorkGroupSizeY_NV
	1, // .maxMeshWorkGroupSizeZ_NV
	32, // .maxTaskWorkGroupSizeX_NV
	1, // .maxTaskWorkGroupSizeY_NV
	1, // .maxTaskWorkGroupSizeZ_NV
	4, // .maxMeshViewCountNV
	1, // maxDualSourceDrawBuffersEXT;
	TLimits{ // limits
		true, // .nonInductiveForLoops
		true, // .whileLoops
		true, // .doWhileLoops
		true, // .generalUniformIndexing
		true, // .generalAttributeMatrixVectorIndexing
		true, // .generalVaryingIndexing
		true, // .generalSamplerIndexing
		true, // .generalVariableIndexing
		true, // .generalConstantMatrixVectorIndexing
	}
};


class Includer : public glslang::TShader::Includer
{
public:
	Includer(const Path* systemDirectories, size_t count) :
		glslang::TShader::Includer(),
		m_systemDirectories(systemDirectories, systemDirectories + count)
	{
	}
	IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
	{
		for (const Path& systemDirectory : m_systemDirectories)
		{
			// TODO Ensure / at the end
			Path header = systemDirectory + headerName;
			if (OS::File::exist(header))
			{
				String str;
				if (OS::File::read(header, &str))
				{
					char* data = new char[str.length() + 1];
					memcpy(data, str.cstr(), str.length() + 1);
					return new IncludeResult(header.cstr(), data, str.length(), nullptr);
				}
			}
		}
		return nullptr;
	}
	IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
	{
		// TODO pass correct path
		Path header = OS::cwd() + headerName;
		if (OS::File::exist(header))
		{
			String str;
			if (OS::File::read(header, &str))
			{
				char* data = new char[str.length() + 1];
				memcpy(data, str.cstr(), str.length() + 1);
				return new IncludeResult(header.cstr(), data, str.length(), nullptr);
			}
		}
		return nullptr;
	}
	void releaseInclude(IncludeResult* result) override
	{
		if (result)
		{
			if (result->headerData)
				delete[] result->headerData;
			delete result;
		}
	}
private:
	std::vector<Path> m_systemDirectories;
};

ShaderCompiler::ShaderCompiler()
{
}

ShaderCompiler::~ShaderCompiler()
{
}

ShaderBlob ShaderCompiler::compile(const ShaderKey& key)
{
	String file;
	String name = OS::File::basename(key.path);
	if (!OS::File::read(key.path, &file))
		return ShaderBlob();

	EShLanguage stage = EShLanguage::EShLangVertex;
	switch (key.type)
	{
	case ShaderType::Vertex:
		stage = EShLangVertex;
		break;
	case ShaderType::Fragment:
		stage = EShLangFragment;
		break;
	case ShaderType::Compute:
		stage = EShLangCompute;
		break;
	case ShaderType::Geometry:
		stage = EShLangGeometry;
		break;
	default:
		return ShaderBlob();
	}

	glslang::InitializeProcess();

	EShMessages messages = EShMsgDefault;
	int default_version = 110; // 110 for desktop
	glslang::TProgram program;
	glslang::TShader shader(stage);

	char* shaderString = file.cstr();
	char* shaderName = name.cstr();
	int shaderLength = (int)file.length();
	shader.setStringsWithLengthsAndNames(&shaderString, &shaderLength, &shaderName, 1);
	// Glslang does not support multiple entry point, but SPIRV supports it.
	// To support it, we would need a different backend.
	//shader.setEntryPoint("");
	shader.setInvertY(false);
	shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, default_version);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

	// Set define values
	std::vector<std::string> processes;
	std::string defs = "#extension GL_GOOGLE_include_directive : require\n";
	// TODO move these defines in key.macro ? as mandatory macros
#if defined(AKA_ORIGIN_TOP_LEFT)
	defs += "#define AKA_FLIP_UV\n";
#endif
#if defined(AKA_ORIGIN_TOP_LEFT)
	defs += "#define AKA_ORIGIN_TOP_LEFT\n";
#elif defined(AKA_ORIGIN_BOTTOM_LEFT)
	defs += "#define AKA_ORIGIN_BOTTOM_LEFT\n";
#endif
	for (const String& string : key.macros)
	{
		defs += "#define " + string + "\n";
		processes.push_back("D" + string);
	}
	shader.setPreamble(defs.c_str());
	shader.addProcesses(processes);

	// Set include directories
	std::vector<Path> paths;
	paths.push_back(key.path.up());
	Includer includer(paths.data(), paths.size());

	if (!shader.parse(&defaultConf, default_version, false, messages, includer))
	{
		Logger::error("Failed to parse shader : ", shader.getInfoLog());
		glslang::FinalizeProcess();
		return ShaderBlob();
	}
	program.addShader(&shader);
	if (!program.link(messages))
	{
		Logger::error("Failed to link shader : ", program.getInfoLog());
		program.~TProgram();
		glslang::FinalizeProcess();
		return ShaderBlob();
	}

	glslang::SpvOptions spv_opts;
	spv_opts.validate = true;
	spv::SpvBuildLogger logger;
	program.getIntermediate(stage);

	std::vector<uint32_t> spirv;
	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &logger, &spv_opts);
	if (!logger.getAllMessages().empty())
	{
		Logger::error(logger.getAllMessages().c_str());
		program.~TProgram();
		glslang::FinalizeProcess();
		return ShaderBlob();
	}
	return ShaderBlob(spirv.data(), spirv.size() * sizeof(uint32_t));
}

gfx::VertexFormat getType(spirv_cross::SPIRType::BaseType type)
{
	switch (type)
	{
	case spirv_cross::SPIRType::BaseType::Half:
		return gfx::VertexFormat::Half;
	case spirv_cross::SPIRType::BaseType::Float:
		return gfx::VertexFormat::Float;
	case spirv_cross::SPIRType::BaseType::UByte:
		return gfx::VertexFormat::UnsignedByte;
	case spirv_cross::SPIRType::BaseType::SByte:
		return gfx::VertexFormat::Byte;
	case spirv_cross::SPIRType::BaseType::UShort:
		return gfx::VertexFormat::UnsignedShort;
	case spirv_cross::SPIRType::BaseType::Short:
		return gfx::VertexFormat::Short;
	case spirv_cross::SPIRType::BaseType::UInt:
		return gfx::VertexFormat::UnsignedInt;
	case spirv_cross::SPIRType::BaseType::Int:
		return gfx::VertexFormat::Int;
	default:
		return gfx::VertexFormat::Unknown;
	}
}
gfx::VertexType getSize(uint32_t rows, uint32_t cols)
{
	switch (cols)
	{
	case 1: {
		switch (rows)
		{
		case 1: return gfx::VertexType::Scalar;
		case 2: return gfx::VertexType::Vec2;
		case 3: return gfx::VertexType::Vec3;
		case 4: return gfx::VertexType::Vec4;
		default: return gfx::VertexType::Unknown;
		}
	}
	case 2: {
		switch (rows)
		{
		case 2: return gfx::VertexType::Mat2;
		default: return gfx::VertexType::Unknown;
		}
	}
	case 3: {
		switch (rows)
		{
		case 3: return gfx::VertexType::Mat3;
		default: return gfx::VertexType::Unknown;
		}
	}
	case 4: {
		switch (rows)
		{
		case 4: return gfx::VertexType::Mat4;
		default: return gfx::VertexType::Unknown;
		}
	}
	default:
		return gfx::VertexType::Unknown;
	}
}

ShaderType getShaderType(spv::ExecutionModel executionModel)
{
	switch (executionModel)
	{
	case spv::ExecutionModelVertex:
		return ShaderType::Vertex;
	case spv::ExecutionModelGeometry:
		return ShaderType::Geometry;
	case spv::ExecutionModelFragment:
		return ShaderType::Fragment;
	case spv::ExecutionModelGLCompute:
		return ShaderType::Compute;
	default:
		return ShaderType::None;
	}
}

ShaderData ShaderCompiler::reflect(const ShaderBlob& blob, const char* entryPoint)
{
	ShaderData data{};
	data.entryPoint = entryPoint;

	// TODO what if blob is HLSL or such
	AKA_ASSERT(blob.size() % 4 == 0, "Invalid size");
	const uint32_t* d = static_cast<const uint32_t*>(blob.data());
	std::vector<uint32_t> spriv(d, d + blob.size() / 4);

	spirv_cross::Compiler compiler(spriv); // TODO cache spirv to avoid reparsing
	spv::ExecutionModel executionModel = compiler.get_execution_model();
	// --- Reflect vertex bindings
	if (executionModel == spv::ExecutionModel::ExecutionModelVertex)
	{
		auto e = compiler.get_entry_point(entryPoint, executionModel);
		for (spirv_cross::VariableID id : e.interface_variables)
		{
			//std::string name = compiler.get_name(id);
			spv::StorageClass storage = compiler.get_storage_class(id);
			if (storage != spv::StorageClass::StorageClassInput)
				continue;
			uint32_t location = compiler.get_decoration(id, spv::Decoration::DecorationLocation);
			spirv_cross::SPIRType type = compiler.get_type_from_variable(id);

			AKA_ASSERT(location < gfx::VertexMaxAttributeCount, "");

			data.vertices.attributes[location].format = getType(type.basetype);
			data.vertices.attributes[location].semantic = gfx::VertexSemantic::Unknown; // store somewhere
			data.vertices.attributes[location].type = getSize(type.vecsize, type.columns);
			data.vertices.count = max(data.vertices.count, location + 1);
		}
		// Compute offsets ? 
		// TODO move them out of here.
		uint32_t offset = 0;
		for (uint32_t i = 0; i < data.vertices.count; i++)
		{
			data.vertices.offsets[i] = offset;
			offset += gfx::VertexBindingState::size(data.vertices.attributes[i].format) * gfx::VertexBindingState::size(data.vertices.attributes[i].type);
		}
	}
	else
	{
		data.vertices = {};
	}
	// --- Reflect shader bindings
	{
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		for (spirv_cross::Resource& resource : resources.sampled_images)
		{
			std::string name = compiler.get_name(resource.id);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			AKA_ASSERT(set < gfx::ShaderMaxSetCount, "not enough set storage.");
			while (set >= data.sets.size()) data.sets.append(gfx::ShaderBindingState{});
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			AKA_ASSERT(binding < gfx::ShaderMaxBindingCount, "not enough binding storage.");
			data.sets[set].bindings[binding].count = 1; // TODO
			data.sets[set].bindings[binding].shaderType = getShaderType(executionModel);
			data.sets[set].bindings[binding].type = gfx::ShaderBindingType::SampledImage;
			data.sets[set].count = max(data.sets[set].count, binding + 1);
		}
		for (spirv_cross::Resource& resource : resources.storage_images)
		{
			std::string name = compiler.get_name(resource.id);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			AKA_ASSERT(set < gfx::ShaderMaxSetCount, "not enough set storage.");
			while (set >= data.sets.size()) data.sets.append(gfx::ShaderBindingState{});
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			AKA_ASSERT(binding < gfx::ShaderMaxBindingCount, "not enough binding storage.");
			data.sets[set].bindings[binding].count = 1; // TODO
			data.sets[set].bindings[binding].shaderType = getShaderType(executionModel);
			data.sets[set].bindings[binding].type = gfx::ShaderBindingType::StorageImage;
			data.sets[set].count = max(data.sets[set].count, binding + 1);
		}
		for (spirv_cross::Resource& resource : resources.uniform_buffers)
		{
			std::string name = compiler.get_name(resource.id);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			AKA_ASSERT(set < gfx::ShaderMaxSetCount, "not enough set storage.");
			while (set >= data.sets.size()) data.sets.append(gfx::ShaderBindingState{});
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			AKA_ASSERT(binding < gfx::ShaderMaxBindingCount, "not enough binding storage.");
			data.sets[set].bindings[binding].count = 1; // TODO
			data.sets[set].bindings[binding].shaderType = getShaderType(executionModel);
			data.sets[set].bindings[binding].type = gfx::ShaderBindingType::UniformBuffer;
			data.sets[set].count = max(data.sets[set].count, binding + 1);
		}
		for (spirv_cross::Resource& resource : resources.storage_buffers)
		{
			std::string name = compiler.get_name(resource.id);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			AKA_ASSERT(set < gfx::ShaderMaxSetCount, "not enough set storage.");
			while (set >= data.sets.size()) data.sets.append(gfx::ShaderBindingState{});
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			AKA_ASSERT(binding < gfx::ShaderMaxBindingCount, "not enough binding storage.");
			data.sets[set].bindings[binding].count = 1; // TODO
			data.sets[set].bindings[binding].shaderType = getShaderType(executionModel);
			data.sets[set].bindings[binding].type = gfx::ShaderBindingType::StorageBuffer;
			data.sets[set].count = max(data.sets[set].count, binding + 1);
		}

		resources.acceleration_structures;
	}
	return data;
}

};