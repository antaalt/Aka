#include <Aka/Graphic/Compiler.h>

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


Compiler::Compiler()
{
}

bool Compiler::parse(const Path& path, ShaderType type, const char** defines, size_t defineCount)
{
	String file;
	String name = OS::File::basename(path);
	if (!OS::File::read(path, &file))
		return false;

	EShLanguage stage = EShLanguage::EShLangVertex;
	switch (type)
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
		return false;
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
	shader.setInvertY(false);
	shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, default_version);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
	shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

	// Set define values
	std::vector<std::string> processes;
	std::string defs = "#extension GL_GOOGLE_include_directive : require\n";
#if defined(AKA_ORIGIN_TOP_LEFT)
	defs += "#define AKA_FLIP_UV\n";
#endif
#if defined(AKA_ORIGIN_TOP_LEFT)
	defs += "#define AKA_ORIGIN_TOP_LEFT\n";
#elif defined(AKA_ORIGIN_BOTTOM_LEFT)
	defs += "#define AKA_ORIGIN_BOTTOM_LEFT\n";
#endif
	for (size_t i = 0; i < defineCount; i++)
	{
		defs += "#define " + std::string(defines[i]) + " 1\n";
		processes.push_back("D" + std::string(defines[i]));
	}
	shader.setPreamble(defs.c_str());
	shader.addProcesses(processes);

	// Set include directories
	std::vector<Path> paths;
	paths.push_back(path.up());
	Includer includer(paths.data(), paths.size());

	if (!shader.parse(&defaultConf, default_version, false, messages, includer))
	{
		Logger::error("Failed to parse shader : ", shader.getInfoLog());
		glslang::FinalizeProcess();
		return false;
	}
	program.addShader(&shader);
	if (!program.link(messages))
	{
		Logger::error("Failed to link shader : ", program.getInfoLog());
		program.~TProgram();
		glslang::FinalizeProcess();
		return false;
	}

	glslang::SpvOptions spv_opts;
	spv_opts.validate = true;
	spv::SpvBuildLogger logger;
	program.getIntermediate(stage);

	glslang::GlslangToSpv(*program.getIntermediate(stage), m_spirv, &logger, &spv_opts);
	if (!logger.getAllMessages().empty())
	{
		Logger::error(logger.getAllMessages().c_str());
		program.~TProgram();
		glslang::FinalizeProcess();
		return false;
	}
	return true;
}

Blob Compiler::compile(GraphicAPI api)
{
	switch (api)
	{
	case aka::GraphicAPI::OpenGL3:
		return compileGLSL330();
	case aka::GraphicAPI::DirectX11:
		VertexBindingState v = getVertexBindings();
		return compileHLSL50(v.attributes, v.count);
	case aka::GraphicAPI::Vulkan:
		return compileSPV();
	default:
		Logger::error("Unsupported API.");
		return Blob();
	}
}

void Compiler::set(const byte_t* bytes, size_t count)
{
	m_spirv = std::vector<uint32_t>(bytes, bytes + count);
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

ShaderBindingState Compiler::getShaderBindings(uint32_t inSet)
{
	ShaderBindingState bindings{};

	spirv_cross::Compiler compiler(m_spirv);
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();
	spv::ExecutionModel executionModel = compiler.get_execution_model();
	for (spirv_cross::Resource& resource : resources.sampled_images)
	{
		std::string name = compiler.get_name(resource.id);
		uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		if (set != inSet)
			continue;
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		AKA_ASSERT(binding < ShaderBindingState::MaxBindingCount, "not enough binding storage.");
		bindings.bindings[binding].count = 1; // TODO
		bindings.bindings[binding].shaderType = getShaderType(executionModel);
		bindings.bindings[binding].type = ShaderBindingType::SampledImage;
		bindings.count = max(bindings.count, binding + 1);
	}
	for (spirv_cross::Resource& resource : resources.storage_images)
	{
		std::string name = compiler.get_name(resource.id);
		uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		if (set != inSet)
			continue;
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		AKA_ASSERT(binding < ShaderBindingState::MaxBindingCount, "not enough binding storage.");
		bindings.bindings[binding].count = 1; // TODO
		bindings.bindings[binding].shaderType = getShaderType(executionModel);
		bindings.bindings[binding].type = ShaderBindingType::StorageImage;
		bindings.count = max(bindings.count, binding + 1);
	}
	for (spirv_cross::Resource& resource : resources.uniform_buffers)
	{
		std::string name = compiler.get_name(resource.id);
		uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		if (set != inSet)
			continue;
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		AKA_ASSERT(binding < ShaderBindingState::MaxBindingCount, "not enough binding storage.");
		bindings.bindings[binding].count = 1; // TODO
		bindings.bindings[binding].shaderType = getShaderType(executionModel);
		bindings.bindings[binding].type = ShaderBindingType::UniformBuffer;
		bindings.count = max(bindings.count, binding + 1);
	}
	for (spirv_cross::Resource& resource : resources.storage_buffers)
	{
		std::string name = compiler.get_name(resource.id);
		uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		if (set != inSet)
			continue;
		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		AKA_ASSERT(binding < ShaderBindingState::MaxBindingCount, "not enough binding storage.");
		bindings.bindings[binding].count = 1; // TODO
		bindings.bindings[binding].shaderType = getShaderType(executionModel);
		bindings.bindings[binding].type = ShaderBindingType::StorageBuffer;
		bindings.count = max(bindings.count, binding + 1);
	}

	resources.acceleration_structures;

	return bindings;
}

VertexFormat getType(spirv_cross::SPIRType::BaseType type)
{
	switch (type)
	{
	case spirv_cross::SPIRType::BaseType::Half:
		return VertexFormat::Half;
	case spirv_cross::SPIRType::BaseType::Float:
		return VertexFormat::Float;
	case spirv_cross::SPIRType::BaseType::UByte:
		return VertexFormat::UnsignedByte;
	case spirv_cross::SPIRType::BaseType::SByte:
		return VertexFormat::Byte;
	case spirv_cross::SPIRType::BaseType::UShort:
		return VertexFormat::UnsignedShort;
	case spirv_cross::SPIRType::BaseType::Short:
		return VertexFormat::Short;
	case spirv_cross::SPIRType::BaseType::UInt:
		return VertexFormat::UnsignedInt;
	case spirv_cross::SPIRType::BaseType::Int:
		return VertexFormat::Int;
	default:
		return VertexFormat::Unknown;
	}
}
VertexType getSize(uint32_t rows, uint32_t cols)
{
	switch (cols)
	{
	case 1: {
		switch (rows)
		{
		case 1: return VertexType::Scalar;
		case 2: return VertexType::Vec2;
		case 3: return VertexType::Vec3;
		case 4: return VertexType::Vec4;
		default: return VertexType::Unknown;
		}
	}
	case 2: {
		switch (rows)
		{
		case 2: return VertexType::Mat2;
		default: return VertexType::Unknown;
		}
	}
	case 3: {
		switch (rows)
		{
		case 3: return VertexType::Mat3;
		default: return VertexType::Unknown;
		}
	}
	case 4: {
		switch (rows)
		{
		case 4: return VertexType::Mat4;
		default: return VertexType::Unknown;
		}
	}
	default:
		return VertexType::Unknown;
	}
}

VertexBindingState Compiler::getVertexBindings()
{
	VertexBindingState bindings{};

	spirv_cross::Compiler compiler(m_spirv); // TODO move compiler to member 
	spv::ExecutionModel executionModel = compiler.get_execution_model();
	if (executionModel != spv::ExecutionModel::ExecutionModelVertex)
		return VertexBindingState{};
	auto e = compiler.get_entry_point("main", executionModel);
	for (spirv_cross::VariableID id : e.interface_variables)
	{
		//std::string name = compiler.get_name(id);
		spv::StorageClass storage = compiler.get_storage_class(id);
		if (storage != spv::StorageClass::StorageClassInput)
			continue;
		uint32_t location = compiler.get_decoration(id, spv::Decoration::DecorationLocation);
		spirv_cross::SPIRType type = compiler.get_type_from_variable(id);
		
		AKA_ASSERT(location < VertexBindingState::MaxAttributes, "");
		
		bindings.attributes[location].format = getType(type.basetype); 
		bindings.attributes[location].semantic = VertexSemantic::Unknown; // store somewhere
		bindings.attributes[location].type =  getSize(type.vecsize, type.columns); 
		bindings.count = max(bindings.count, location + 1);
	}
	// Compute offsets ? 
	// TODO move them out of here.
	uint32_t offset = 0;
	for (uint32_t i = 0; i < bindings.count; i++)
	{
		bindings.offsets[i] = offset;
		offset += VertexBindingState::size(bindings.attributes[i].format) * VertexBindingState::size(bindings.attributes[i].type);
	}
	return bindings;
}

Blob Compiler::compileHLSL50(const VertexAttribute* attributes, size_t count)
{
	spirv_cross::CompilerHLSL compiler(m_spirv);

	spirv_cross::CompilerHLSL::Options optionsHLSL = compiler.get_hlsl_options();
	optionsHLSL.shader_model = 50;
	optionsHLSL.point_size_compat = true;
	optionsHLSL.point_coord_compat = true;

	compiler.set_hlsl_options(optionsHLSL);

	uint32_t new_builtin = compiler.remap_num_workgroups_builtin();
	if (new_builtin) {
		compiler.set_decoration(new_builtin, spv::DecorationDescriptorSet, 0);
		compiler.set_decoration(new_builtin, spv::DecorationBinding, 0);
	}
	// Only for vertex
	for (size_t i = 0; i < count; i++) {
		std::string semantic = "TEXCOORD";
		switch (attributes[i].semantic)
		{
		case VertexSemantic::Position: semantic = "POSITION"; break;
		case VertexSemantic::Normal: semantic = "NORMAL"; break;
		case VertexSemantic::Tangent: semantic = "TANGENT"; break;
		case VertexSemantic::TexCoord0: semantic = "TEXCOORD0"; break;
		case VertexSemantic::TexCoord1: semantic = "TEXCOORD1"; break;
		case VertexSemantic::TexCoord2: semantic = "TEXCOORD2"; break;
		case VertexSemantic::TexCoord3: semantic = "TEXCOORD3"; break;
		case VertexSemantic::Color0: semantic = "COLOR0"; break;
		case VertexSemantic::Color1: semantic = "COLOR1"; break;
		case VertexSemantic::Color2: semantic = "COLOR2"; break;
		case VertexSemantic::Color3: semantic = "COLOR3"; break;
		default: Logger::warn("Semantic not supported."); break;
		}
		spirv_cross::HLSLVertexAttributeRemap remap = { (uint32_t)i, semantic };
		compiler.add_vertex_attribute_remap(remap);
	}
	std::string str = compiler.compile();
	return Blob(reinterpret_cast<byte_t*>(str.data()), str.size());
}

Blob Compiler::compileGLSL330()
{
	spirv_cross::CompilerGLSL compiler(m_spirv);

	spirv_cross::CompilerGLSL::Options options = compiler.get_common_options();
	options.enable_420pack_extension = false;
	options.es = false;
	options.version = 330;

	compiler.set_common_options(options);

	std::string str = compiler.compile();
	return Blob(reinterpret_cast<byte_t*>(str.data()), str.size());
}

Blob Compiler::compileSPV()
{
	return Blob(reinterpret_cast<byte_t*>(m_spirv.data()), m_spirv.size() * sizeof(uint32_t));
}


};
