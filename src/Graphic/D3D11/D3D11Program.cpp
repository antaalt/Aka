#if defined(AKA_USE_D3D11)
#include "D3D11Program.h"

#include "D3D11Context.h"
#include "D3D11Device.h"

#include <Aka/OS/Logger.h>

namespace aka {

D3D11Shader::D3D11Shader(ID3D10Blob* shaderBuffer) :
	Shader(),
	m_shaderBuffer(shaderBuffer)
{
}
D3D11Shader::~D3D11Shader()
{
	if (m_shaderBuffer)
		m_shaderBuffer->Release();
}

Shader::Ptr D3D11Shader::compileHLSL(const char* content, ShaderType type)
{
	ID3DBlob* shaderBuffer = nullptr;
	ID3DBlob* errorMessage = nullptr;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(AKA_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif
	std::string entryPoint;
	std::string version;
	switch (type)
	{
	case ShaderType::Vertex:
		entryPoint = "main";
		version = "vs_5_0";
		break;
	case ShaderType::Fragment:
		entryPoint = "main";
		version = "ps_5_0";
		break;
	case ShaderType::Geometry:
		entryPoint = "main";
		version = "gs_5_0";
		break;
	case ShaderType::Compute:
		entryPoint = "main";
		version = "cs_5_0";
		break;
	default:
		Logger::error("Shader type not supported");
		break;
	}
	// Compile from command line instead
	HRESULT result = D3DCompile(
		content,
		strlen(content),
		nullptr, // error string
		nullptr,
		nullptr,
		entryPoint.c_str(),
		version.c_str(),
		flags,
		0,
		&shaderBuffer,
		&errorMessage
	);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have written something to the error message.
		if (errorMessage)
		{
			// Get a pointer to the error message text buffer.
			char* compileErrors = (char*)(errorMessage->GetBufferPointer());
			// Get the length of the message.
			SIZE_T bufferSize = errorMessage->GetBufferSize();
			Logger::error("[compilation] ", compileErrors);
			// Release the error message.
			errorMessage->Release();
			errorMessage = 0;
			return nullptr;
		}
		else
		{
			Logger::error("[compilation] Missing shader file");
			return nullptr;
		}
	}
	return std::make_shared<D3D11Shader>(shaderBuffer);
}

std::vector<Uniform> getUniforms(ID3D10Blob* shader, ShaderType shaderType)
{
	if (shader == nullptr)
		return std::vector<Uniform>();
	std::vector<Uniform> uniforms;
	ID3D11ShaderReflection* reflector = nullptr;
	D3D_CHECK_RESULT(D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector));

	D3D11_SHADER_DESC shader_desc{};
	D3D_CHECK_RESULT(reflector->GetDesc(&shader_desc));

	// --- Textures
	for (uint32_t i = 0; i < shader_desc.BoundResources; i++)
	{
		D3D11_SHADER_INPUT_BIND_DESC desc{};
		D3D_CHECK_RESULT(reflector->GetResourceBindingDesc(i, &desc));

		if (desc.Type == D3D_SIT_TEXTURE)
		{
			// TODO add support for texture array instead of this ugly hack
			String name = desc.Name;
			uint32_t count = 1;
			if (name[name.length() - 1] == ']' && name[name.length() - 3] == '[')
			{
				uint32_t id = name[name.length() - 2] - '0';
				name = name.substr(0, name.length() - 3);
				if (id > 0)
				{
					for (Uniform& uniform : uniforms)
					{
						if (uniform.name == name)
						{
							uniform.count++;
							break;
						}
					}
					continue;
				}
			}
			if (desc.Dimension == D3D_SRV_DIMENSION_TEXTURE2D)
			{
				uniforms.emplace_back();
				Uniform& uniform = uniforms.back();
				uniform.name = name;
				uniform.shaderType = shaderType;
				uniform.count = max(count, desc.BindCount);
				uniform.type = UniformType::Texture2D;
				uniform.binding = desc.BindPoint;
			}
			else if (desc.Dimension == D3D_SRV_DIMENSION_TEXTURECUBE)
			{
				uniforms.emplace_back();
				Uniform& uniform = uniforms.back();
				uniform.name = name;
				uniform.shaderType = shaderType;
				uniform.count = max(count, desc.BindCount);
				uniform.type = UniformType::TextureCubemap;
				uniform.binding = desc.BindPoint;
			}
		}
		else if (desc.Type == D3D_SIT_CBUFFER)
		{
			uniforms.emplace_back();
			Uniform& uniform = uniforms.back();
			uniform.name = desc.Name;
			uniform.shaderType = shaderType;
			uniform.count = max(1U, desc.BindCount);
			uniform.type = UniformType::Buffer;
			uniform.binding = desc.BindPoint;
		}
		// Samplers are automatically created for each textures (as in GLSL, they are implicit for each texture)
		/*else if (desc.Type == D3D_SIT_SAMPLER)
		{
			uniforms.emplace_back();
			Uniform& uniform = uniforms.back();
			uniform.name = desc.Name;
			uniform.shaderType = shaderType;
			uniform.count = max(1U, desc.BindCount);
			uniform.type = UniformType::Sampler2D;
		}*/
	}
	return uniforms;
}

D3D11Program::D3D11Program(D3D11Device* device, Shader::Ptr vertex, Shader::Ptr fragment, Shader::Ptr geometry, Shader::Ptr compute, const VertexAttribute* attributes, size_t count) :
	Program(attributes, count),
	m_device(device),
	m_layout(nullptr),
	m_vertexShader(nullptr),
	m_pixelShader(nullptr),
	m_geometryShader(nullptr),
	m_computeShader(nullptr)
{
	auto getShaderBuffer = [](Shader* shader) -> ID3D10Blob* {
		if (shader == nullptr)
			return nullptr;
		return reinterpret_cast<D3D11Shader*>(shader)->m_shaderBuffer;
	};
	ID3D10Blob* vertexShaderBuffer = getShaderBuffer(vertex.get());
	ID3D10Blob* pixelShaderBuffer = getShaderBuffer(fragment.get());
	ID3D10Blob* geometryShaderBuffer = getShaderBuffer(geometry.get());
	ID3D10Blob* computeShaderBuffer = getShaderBuffer(compute.get());
	// --- Create shaders
	if (vertex != nullptr)
		D3D_CHECK_RESULT(m_device->device()->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader));
	if (fragment != nullptr)
		D3D_CHECK_RESULT(m_device->device()->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &m_pixelShader));
	if (compute != nullptr)
		D3D_CHECK_RESULT(m_device->device()->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), nullptr, &m_computeShader));
	if (geometry != nullptr)
		D3D_CHECK_RESULT(m_device->device()->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), nullptr, &m_geometryShader));

	// --- Layout
	if (compute == nullptr)
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> polygonLayout(count);
		AKA_ASSERT(m_attributes.size() == count, "Incorrect size");
		for (uint32_t i = 0; i < count; i++)
		{
			const VertexAttribute& a = attributes[i];
			switch (a.semantic)
			{
			case VertexSemantic::Position: polygonLayout[i].SemanticName = "POSITION"; break;
			case VertexSemantic::Normal: polygonLayout[i].SemanticName = "NORMAL"; break;
			case VertexSemantic::Tangent: polygonLayout[i].SemanticName = "TANGENT"; break;
			case VertexSemantic::TexCoord0: polygonLayout[i].SemanticName = "TEXCOORD"; break;
			case VertexSemantic::TexCoord1: polygonLayout[i].SemanticName = "TEXCOORD"; break;
			case VertexSemantic::TexCoord2: polygonLayout[i].SemanticName = "TEXCOORD"; break;
			case VertexSemantic::TexCoord3: polygonLayout[i].SemanticName = "TEXCOORD"; break;
			case VertexSemantic::Color0: polygonLayout[i].SemanticName = "COLOR"; break;
			case VertexSemantic::Color1: polygonLayout[i].SemanticName = "COLOR"; break;
			case VertexSemantic::Color2: polygonLayout[i].SemanticName = "COLOR"; break;
			case VertexSemantic::Color3: polygonLayout[i].SemanticName = "COLOR"; break;
			default: Logger::error("Semantic not supported");
				break;
			}
			polygonLayout[i].SemanticIndex = 0; // TODO if mat4, set different index
			if (a.format == VertexFormat::Float)
			{
				switch (a.type)
				{
				case VertexType::Scalar: polygonLayout[i].Format = DXGI_FORMAT_R32_FLOAT;  break;
				case VertexType::Vec2: polygonLayout[i].Format = DXGI_FORMAT_R32G32_FLOAT; break;
				case VertexType::Vec3: polygonLayout[i].Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
				case VertexType::Vec4: polygonLayout[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
				default: Logger::error("Format not supported for layout");
				}
			}
			else if (a.format == VertexFormat::Byte)
			{
				switch (a.type)
				{
				case VertexType::Vec4: polygonLayout[i].Format = DXGI_FORMAT_R8G8B8A8_SNORM; break;
				default: Logger::error("Format not supported for layout");
				}
			}
			else if (a.format == VertexFormat::UnsignedByte)
			{
				switch (a.type)
				{
				case VertexType::Vec4: polygonLayout[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
				default: Logger::error("Format not supported for layout");
				}
			}
			else if (a.format == VertexFormat::Short)
			{
				switch (a.type)
				{
				case VertexType::Vec4: polygonLayout[i].Format = DXGI_FORMAT_R16G16B16A16_SNORM; break;
				default: Logger::error("Format not supported for layout");
				}
			}
			else if (a.format == VertexFormat::UnsignedShort)
			{
				switch (a.type)
				{
				case VertexType::Vec4: polygonLayout[i].Format = DXGI_FORMAT_R16G16B16A16_UNORM; break;
				default: Logger::error("Format not supported for layout");
				}
			}
			else if (a.format == VertexFormat::Int)
			{
				switch (a.type)
				{
				case VertexType::Vec4: polygonLayout[i].Format = DXGI_FORMAT_R32G32B32A32_SINT; break;
				default: Logger::error("Format not supported for layout");
				}
			}
			else if (a.format == VertexFormat::UnsignedInt)
			{
				switch (a.type)
				{
				case VertexType::Vec4: polygonLayout[i].Format = DXGI_FORMAT_R32G32B32A32_UINT; break;
				default: Logger::error("Format not supported for layout");
				}
			}
			else
			{
				Logger::error("Format not supported for layout");
			}
			polygonLayout[i].InputSlot = 0;
			polygonLayout[i].AlignedByteOffset = (i == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT);
			polygonLayout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			polygonLayout[i].InstanceDataStepRate = 0;
		}
		// TODO cache layout
		D3D_CHECK_RESULT(m_device->device()->CreateInputLayout(
			polygonLayout.data(),
			(UINT)polygonLayout.size(),
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			&m_layout
		));
	}

	// --- Uniforms
	// Find and merge all uniforms
	std::vector<Uniform> uniformsVert = getUniforms(vertexShaderBuffer, ShaderType::Vertex);
	std::vector<Uniform> uniformsFrag = getUniforms(pixelShaderBuffer, ShaderType::Fragment);
	std::vector<Uniform> uniformsGeo = getUniforms(geometryShaderBuffer, ShaderType::Geometry);
	std::vector<Uniform> uniformsComp = getUniforms(computeShaderBuffer, ShaderType::Compute);
	m_uniforms.insert(m_uniforms.end(), uniformsVert.begin(), uniformsVert.end());
	m_uniforms.insert(m_uniforms.end(), uniformsFrag.begin(), uniformsFrag.end());
	m_uniforms.insert(m_uniforms.end(), uniformsGeo.begin(), uniformsGeo.end());
	m_uniforms.insert(m_uniforms.end(), uniformsComp.begin(), uniformsComp.end());
	// Remove duplicates
	for (size_t i = 0; i < m_uniforms.size(); i++)
	{
		for (size_t j = i + 1; j < m_uniforms.size(); j++)
		{
			if (m_uniforms[i].name == m_uniforms[j].name)
			{
				if (m_uniforms[i].type == m_uniforms[j].type)
				{
					m_uniforms[i].shaderType = (ShaderType)((int)m_uniforms[i].shaderType | (int)m_uniforms[j].shaderType);
					m_uniforms.erase(m_uniforms.begin() + j);
					j--;
				}
			}
		}
	}
}
D3D11Program::~D3D11Program()
{
	if (m_layout)
		m_layout->Release();
	if (m_pixelShader)
		m_pixelShader->Release();
	if (m_vertexShader)
		m_vertexShader->Release();
	if (m_geometryShader)
		m_geometryShader->Release();
	if (m_computeShader)
		m_computeShader->Release();
}

void D3D11Program::use()
{
	m_device->context()->IASetInputLayout(m_layout);
	m_device->context()->VSSetShader(m_vertexShader, nullptr, 0);
	m_device->context()->GSSetShader(m_geometryShader, nullptr, 0);
	m_device->context()->PSSetShader(m_pixelShader, nullptr, 0);
	m_device->context()->CSSetShader(m_computeShader, nullptr, 0);
}

};

#endif