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
// Should be read from device capabilities...
// But would require runtime compilation...
// from https://github.com/KhronosGroup/glslang/blob/main/glslang/ResourceLimits/ResourceLimits.cpp
// Should adapt this list to our need.
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
	256, // .maxMeshOutputVerticesEXT
	256, // .maxMeshOutputPrimitivesEXT
	128, // .maxMeshWorkGroupSizeX_EXT
	128, // .maxMeshWorkGroupSizeY_EXT
	128, // .maxMeshWorkGroupSizeZ_EXT
	128, // .maxTaskWorkGroupSizeX_EXT
	128, // .maxTaskWorkGroupSizeY_EXT
	128, // .maxTaskWorkGroupSizeZ_EXT
	4, // .maxMeshViewCountEXT
	1, // .maxDualSourceDrawBuffersEXT;
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
	Includer(const Path* systemDirectories, size_t count, AssetPathType assetPathType) :
		glslang::TShader::Includer(),
		m_assetPathType(assetPathType),
		m_systemDirectories(systemDirectories, count)
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
				// Compute relative path & register the dependency.
				Path relativePath = OS::relative(header, AssetPath("", m_assetPathType).getAbsolutePath());
				// Remove ./ at beginning of path.
				if (relativePath.size() > 2 && relativePath[0] == '.' && relativePath[1] == '/')
				{
					relativePath = Path(relativePath.string().substr(2, relativePath.size() - 2));
				}
				AssetPath assetPath = AssetPath(relativePath.cstr(), m_assetPathType);
				m_dependencies.append(assetPath);
				// Read file
				String str;
				if (OS::File::read(header, &str))
				{
					// Destroyed in releaseInclude
					char* data = mem::akaNewArray<char>(str.length() + 1, AllocatorMemoryType::String, AllocatorCategory::Graphic);
					Memory::copy(data, str.cstr(), str.length() + 1);
					return mem::akaNew<IncludeResult>(AllocatorMemoryType::String, AllocatorCategory::Graphic, header.cstr(), data, str.length(), nullptr);
				}
			}
		}
		return nullptr;
	}
	IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
	{
		Path header = OS::cwd() + headerName;
		if (OS::File::exist(header))
		{
			// Compute relative path & register the dependency.
			Path relativePath = OS::relative(header, AssetPath("", m_assetPathType).getAbsolutePath());
			// Remove ./ at beginning of path.
			if (relativePath.size() > 2 && relativePath[0] == '.' && relativePath[1] == '/')
			{
				relativePath = Path(relativePath.string().substr(2, relativePath.size() - 2));
			}
			AssetPath assetPath = AssetPath(relativePath.cstr(), m_assetPathType);
			m_dependencies.append(assetPath);
			// Read file
			String str;
			if (OS::File::read(header, &str))
			{
				// Destroyed in releaseInclude
				char* data = mem::akaNewArray<char>(str.length() + 1, AllocatorMemoryType::String, AllocatorCategory::Graphic);
				Memory::copy(data, str.cstr(), str.length() + 1);
				return mem::akaNew<IncludeResult>(AllocatorMemoryType::String, AllocatorCategory::Graphic, header.cstr(), data, str.length(), nullptr);
			}
		}
		return nullptr;
	}
	void releaseInclude(IncludeResult* result) override
	{
		if (result)
		{
			if (result->headerData)
				mem::akaDeleteArray<char>(result->headerData);
			mem::akaDelete<IncludeResult>(result);
		}
	}
	const Vector<AssetPath>& getDependencies() const {
		return m_dependencies;
	}
private:
	AssetPathType m_assetPathType;
	Vector<Path> m_systemDirectories;
	Vector<AssetPath> m_dependencies;
};

ShaderCompiler::ShaderCompiler()
{
	// This took some megabytes in footprint but greatly increase shader compilation time.
	glslang::InitializeProcess();
}

ShaderCompiler::~ShaderCompiler()
{
	glslang::FinalizeProcess();
}

ShaderCompilationResult ShaderCompiler::compile(const ShaderKey& key)
{
	// OPTIM: compile multiple shader of same stage at the same time for improved perfs.
	String file;
	String name = OS::File::basename(key.path.getAbsolutePath());
	if (!OS::File::read(key.path.getAbsolutePath(), &file))
		return ShaderCompilationResult();

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
	case ShaderType::Task:
		stage = EShLangTask;
		break;
	case ShaderType::Mesh:
		stage = EShLangMesh;
		break;
	default:
		return ShaderCompilationResult();
	}
	
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
	shader.setEntryPoint(key.entryPoint.cstr());
	shader.setInvertY(false);
	shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, default_version);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3); // Minimum required VK version
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_4); // Minimum required for mesh shader EXT

	// Set define values
	std::vector<std::string> processes;
	// TODO: check if define does not already exist.
	String defs = "#extension GL_GOOGLE_include_directive : require\n";
	// TODO: move these defines in key.macro ? as mandatory macros
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
		processes.push_back("D" + std::string(string.data(), string.size()));
	}
	shader.setPreamble(defs.cstr());
	shader.addProcesses(processes);

	// Set include directories
	Vector<Path> paths;
	paths.append(key.path.getAbsolutePath().up());
	Includer includer(paths.data(), paths.size(), key.path.getType());

	if (!shader.parse(&defaultConf, default_version, false, messages, includer))
	{
		Logger::error("Failed to parse shader ", key, " : ", shader.getInfoLog());
		return ShaderCompilationResult();
	}
	program.addShader(&shader);
	if (!program.link(messages))
	{
		Logger::error("Failed to link shader : ", program.getInfoLog());
		return ShaderCompilationResult();
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
		return ShaderCompilationResult();
	}
	return ShaderCompilationResult { ShaderBlob(spirv.data(), spirv.size() * sizeof(uint32_t)), includer.getDependencies() };
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
		AKA_ASSERT(false, "");
		return gfx::VertexFormat::Unknown;
	}
}
gfx::VertexType getSize(uint32_t rows)
{
	switch (rows)
	{
	case 1: return gfx::VertexType::Scalar;
	case 2: return gfx::VertexType::Vec2;
	case 3: return gfx::VertexType::Vec3;
	case 4: return gfx::VertexType::Vec4;
		AKA_ASSERT(false, "");
	default: return gfx::VertexType::Unknown;
	}
}

gfx::ShaderMask getShaderMask(spv::ExecutionModel executionModel)
{
	switch (executionModel)
	{
	case spv::ExecutionModelVertex:
		return gfx::ShaderMask::Vertex;
	case spv::ExecutionModelFragment:
		return gfx::ShaderMask::Fragment;
	case spv::ExecutionModelTaskEXT:
		return gfx::ShaderMask::Task;
	case spv::ExecutionModelMeshEXT:
		return gfx::ShaderMask::Mesh;
	case spv::ExecutionModelGLCompute:
		return gfx::ShaderMask::Compute;
	default:
		return gfx::ShaderMask::None;
	}
}

ShaderReflectionData ShaderCompiler::reflect(const ShaderCompilationResult& compilationResult, const char* entryPoint)
{
	const ShaderBlob& blob = compilationResult.blob;
	ShaderReflectionData data{};
	data.entryPoint = entryPoint;
	data.dependencies = compilationResult.dependencies;

	// TODO what if blob is HLSL or such
	AKA_ASSERT(blob.size() % 4 == 0, "Invalid size");
	const uint32_t* d = static_cast<const uint32_t*>(blob.data());
	std::vector<uint32_t> spirv(d, d + blob.size() / 4);
	try
	{
		spirv_cross::Compiler compiler(spirv); // TODO cache spirv to avoid reparsing
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
				uint32_t binding = compiler.get_decoration(id, spv::Decoration::DecorationBinding);
				spirv_cross::SPIRType type = compiler.get_type_from_variable(id);

				AKA_ASSERT(location < gfx::VertexMaxAttributeCount, "");
				// Each column uses a single location
				for (uint32_t iCol = 0; iCol < type.columns; iCol++)
				{
					data.vertices.bufferLayout[binding].attributes[location + iCol].format = getType(type.basetype);
					data.vertices.bufferLayout[binding].attributes[location + iCol].semantic = gfx::VertexSemantic::Unknown; // store somewhere
					data.vertices.bufferLayout[binding].attributes[location + iCol].type = getSize(type.vecsize);
				}
				data.vertices.bufferLayout[binding].count = max(data.vertices.bufferLayout[binding].count, location + type.columns);
				data.vertices.bufferLayout[binding].stepRate = gfx::VertexStepRate::Unknown;
				data.vertices.count = max(data.vertices.count, binding + 1);
			}
			// Compute offsets ? 
			// TODO move them out of here.
			for (uint32_t iBuffer = 0; iBuffer < data.vertices.count; iBuffer++)
			{
				uint32_t offset = 0; // per buffer
				gfx::VertexBufferLayout layout = data.vertices.bufferLayout[iBuffer];
				for (uint32_t iBinding = 0; iBinding < layout.count; iBinding++)
				{
					layout.offsets[iBinding] = offset;
					offset += gfx::VertexBufferLayout::size(layout.attributes[iBinding].format) * gfx::VertexBufferLayout::size(layout.attributes[iBinding].type);
				}
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
				data.sets[set].bindings[binding].stages = getShaderMask(executionModel);
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
				data.sets[set].bindings[binding].stages = getShaderMask(executionModel);
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
				data.sets[set].bindings[binding].stages = getShaderMask(executionModel);
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
				data.sets[set].bindings[binding].stages = getShaderMask(executionModel);
				data.sets[set].bindings[binding].type = gfx::ShaderBindingType::StorageBuffer;
				data.sets[set].count = max(data.sets[set].count, binding + 1);
			}

			resources.acceleration_structures;
		}
		{ // --- Reflect shader constants

			spirv_cross::ShaderResources resources = compiler.get_shader_resources();
			for (spirv_cross::Resource& resource : resources.push_constant_buffers)
			{
				std::string name = compiler.get_name(resource.id);
				auto ranges = compiler.get_active_buffer_ranges(resource.id);
				for (auto& range : ranges)
				{
					AKA_ASSERT(range.index < gfx::ShaderMaxConstantCount, "Too many shader constants");
					while (range.index >= data.constants.size()) data.constants.append(gfx::ShaderConstant{});
					data.constants[range.index].offset = static_cast<uint32_t>(range.offset);
					data.constants[range.index].size = static_cast<uint32_t>(range.range);
					data.constants[range.index].shader = getShaderMask(executionModel);
				}
			}
		}
		return data;
	}
	catch (const spirv_cross::CompilerError& e)
	{
		Logger::error("Failed to compile shader : ", e.what());
		return ShaderReflectionData{};
	}
}

};