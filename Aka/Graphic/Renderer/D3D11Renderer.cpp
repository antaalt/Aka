#if defined(AKA_USE_D3D11)
#include "D3D11Renderer.h"
#include "../GraphicBackend.h"
#include "../../Core/Debug.h"
#include "../../Platform/Platform.h"
#include "../../Platform/Logger.h"
#include "../../Platform/Window.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <stdexcept>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define D3D_CHECK_RESULT(result)    \
{                                   \
    HRESULT res = (result);         \
    if (FAILED(res)) {              \
        char buffer[256];           \
        snprintf(                   \
            buffer,                 \
            256,                    \
            "%s (%s at %s:%d)",     \
            std::system_category(). \
			message(res).c_str(),   \
            STRINGIFY(result),      \
            __FILE__,               \
            __LINE__				\
        );							\
        ::aka::Logger::error(buffer);   \
        throw std::runtime_error(buffer);\
	}								\
}

namespace aka {

struct D3D11SwapChain {
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11DepthStencilState* depthStencilState;
	ID3D11Texture2D* depthStencilBuffer;
	bool vsync = true;
	bool fullscreen = false;
	ID3D11RasterizerState* rasterState;
};

D3D11Context ctx;
D3D11SwapChain swapChain;

class D3D11Texture : public Texture
{
public:
	D3D11Texture(uint32_t width, uint32_t height, Format format, const uint8_t* data, Sampler::Filter filter) :
		Texture(width, height),
		m_texture(nullptr),
		m_view(nullptr)
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // BIND_RENDER_TARGET
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		//if (is_framebuffer)
		//	desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

		switch (format)
		{
		case Texture::Format::Red:
			desc.Format = DXGI_FORMAT_R8_UNORM;
			break;
		case Texture::Format::Rgba:
		case Texture::Format::Rgba8:
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case Texture::Format::DepthStencil:
			desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			break;
		default:
			Logger::error("Format not supported");
			break;
		}

		D3D_CHECK_RESULT(ctx.device->CreateTexture2D(&desc, nullptr, &m_texture));
		D3D_CHECK_RESULT(ctx.device->CreateShaderResourceView(m_texture, nullptr, &m_view));
		// TODO upload data
	}
	D3D11Texture(D3D11Texture&) = delete;
	D3D11Texture& operator=(D3D11Texture&) = delete;
	~D3D11Texture()
	{
		if (m_view)
			m_view->Release();
		if (m_texture)
			m_texture->Release();
	}
	void bind() override
	{
		throw std::runtime_error("Not implemented");
	}
	Handle handle() override
	{
		return Handle((uintptr_t)m_view);
	}
private:
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_view;
};

class D3D11Framebuffer : public Framebuffer
{
public:
	D3D11Framebuffer(uint32_t width, uint32_t height, Attachment* attachment, size_t count) :
		Framebuffer(width, height),
		m_renderTargetView(nullptr),
		m_depthStencilView(nullptr)
	{
	}
	D3D11Framebuffer(const D3D11Framebuffer&) = delete;
	D3D11Framebuffer& operator=(const D3D11Framebuffer&) = delete;
	~D3D11Framebuffer()
	{

	}
	void resize(uint32_t width, uint32_t height) override
	{

	}
	void clear(float r, float g, float b, float a) override
	{
		// clear
		float color[4];
		color[0] = r;
		color[1] = g;
		color[2] = b;
		color[3] = a;
		// Clear the back buffer.
		ctx.deviceContext->ClearRenderTargetView(m_renderTargetView, color);
		// Clear the depth buffer.
		ctx.deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	void bind(Type type) override
	{

	}
	void blit(Framebuffer::Ptr dst, const Rect& srcRect, const Rect& dstRect, Sampler::Filter filter) override
	{

	}
private:
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;
};

class D3D11BackBuffer : public Framebuffer
{
public:
	D3D11BackBuffer(uint32_t width, uint32_t height) :
		Framebuffer(width, height),
		m_renderTargetView(nullptr),
		m_depthStencilView(nullptr)
	{
	}
	D3D11BackBuffer(const D3D11BackBuffer&) = delete;
	D3D11BackBuffer& operator=(const D3D11BackBuffer&) = delete;
	~D3D11BackBuffer()
	{

	}
	void resize(uint32_t width, uint32_t height) override
	{

	}
	void clear(float r, float g, float b, float a) override
	{
		// clear
		float color[4];
		color[0] = r;
		color[1] = g;
		color[2] = b;
		color[3] = a;
		// Clear the back buffer.
		ctx.deviceContext->ClearRenderTargetView(m_renderTargetView, color);
		// Clear the depth buffer.
		ctx.deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	void bind(Type type) override
	{

	}
	void blit(Framebuffer::Ptr dst, const Rect& srcRect, const Rect& dstRect, Sampler::Filter filter) override
	{

	}
private:
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;
};

class D3D11Mesh : public Mesh
{
public:
	D3D11Mesh() :
		Mesh(),
		m_indexBuffer(nullptr),
		m_vertexBuffer(nullptr)
	{

	}
	D3D11Mesh(const D3D11Mesh&) = delete;
	D3D11Mesh& operator=(const D3D11Mesh&) = delete;
	~D3D11Mesh()
	{
		if (m_indexBuffer)
			m_indexBuffer->Release();
		if (m_vertexBuffer)
			m_vertexBuffer->Release();
	}
public:
	void vertices(const VertexData& vertex, const void* vertices, size_t count) override
	{
		m_vertexData = vertex;
		m_vertexCount = static_cast<uint32_t>(count);
		m_vertexStride = 0;
		for (const VertexData::Attribute& attribute : vertex.attributes)
		{
			switch (attribute.type)
			{
			case VertexFormat::Float: m_vertexStride += 4; break;
			case VertexFormat::Float2: m_vertexStride += 8; break;
			case VertexFormat::Float3: m_vertexStride += 12; break;
			case VertexFormat::Float4: m_vertexStride += 16; break;
			case VertexFormat::Byte4: m_vertexStride += 4; break;
			case VertexFormat::Ubyte4: m_vertexStride += 4; break;
			case VertexFormat::Short2: m_vertexStride += 4; break;
			case VertexFormat::Ushort2: m_vertexStride += 4; break;
			case VertexFormat::Short4: m_vertexStride += 8; break;
			case VertexFormat::Ushort4: m_vertexStride += 8; break;
			}
		}

		D3D11_BUFFER_DESC vertexBufferDesc{};
		// Set up the description of the static vertex buffer.
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = static_cast<uint32_t>(m_vertexStride * count);
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		D3D11_SUBRESOURCE_DATA vertexData{};
		vertexData.pSysMem = vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Now create the vertex buffer.
		D3D_CHECK_RESULT(ctx.device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer));
	}

	void indices(IndexFormat indexFormat, const void* indices, size_t count) override
	{
		m_indexCount = static_cast<uint32_t>(count);
		switch (indexFormat)
		{
		case IndexFormat::Uint8:
			// TODO check for support
			m_indexFormat = IndexFormat::Uint8;
			m_indexSize = 1;
			break;
		case IndexFormat::Uint16:
			m_indexFormat = IndexFormat::Uint16;
			m_indexSize = 2;
			break;
		case IndexFormat::Uint32:
			m_indexFormat = IndexFormat::Uint32;
			m_indexSize = 4;
			break;
		}
		// Set up the description of the static index buffer.
		D3D11_BUFFER_DESC indexBufferDesc{};
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = static_cast<uint32_t>(m_indexSize * count);
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the index data.
		D3D11_SUBRESOURCE_DATA indexData{};
		indexData.pSysMem = indices;
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		// Create the index buffer.
		D3D_CHECK_RESULT(ctx.device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer));
	}
	void draw(uint32_t indexCount, uint32_t indexOffset) const override
	{
		unsigned int offset = 0;
		// Set the vertex buffer to active in the input assembler so it can be rendered.
		ctx.deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_vertexStride, &offset);
		// Set the index buffer to active in the input assembler so it can be rendered.
		ctx.deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		ctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// Draw indexed primitives
		ctx.deviceContext->DrawIndexed(indexCount, indexOffset, 0);
	}
private:
	ID3D11Buffer* m_indexBuffer;
	ID3D11Buffer* m_vertexBuffer;
};

class D3D11Shader : public Shader
{
public:
	D3D11Shader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes) :
		Shader(attributes),
		m_layout(nullptr),
		m_vertexShader(nullptr),
		m_pixelShader(nullptr),
		m_computeShader(nullptr),
		m_vertexShaderBuffer((ID3D10Blob*)vert.value()),
		m_pixelShaderBuffer((ID3D10Blob*)frag.value()),
		m_computeShaderBuffer((ID3D10Blob*)compute.value())
	{
		if (vert.value() != 0)
			D3D_CHECK_RESULT(ctx.device->CreateVertexShader(m_vertexShaderBuffer->GetBufferPointer(), m_vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader));
		if (frag.value() != 0)
			D3D_CHECK_RESULT(ctx.device->CreatePixelShader(m_pixelShaderBuffer->GetBufferPointer(), m_pixelShaderBuffer->GetBufferSize(), nullptr, &m_pixelShader));
		if (compute.value() != 0)
			D3D_CHECK_RESULT(ctx.device->CreateComputeShader(m_computeShaderBuffer->GetBufferPointer(), m_computeShaderBuffer->GetBufferSize(), nullptr, &m_computeShader));

		getUniforms(m_vertexShaderBuffer, m_vertexUniformBuffers, ShaderType::Vertex);
		getUniforms(m_pixelShaderBuffer, m_fragmentUniformBuffers, ShaderType::Fragment);

		// combine uniforms that were in both
		// TODO check for same buffer index ?
		for (size_t i = 0; i < m_uniforms.size(); i++)
		{
			m_uniforms[i].id = UniformID(i);
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
	D3D11Shader(const D3D11Shader&) = delete;
	D3D11Shader& operator=(const D3D11Shader&) = delete;
	~D3D11Shader()
	{
		//if (m_uniformBuffer)
		//	m_uniformBuffer->Release();
		if (m_layout)
			m_layout->Release();
		if (m_pixelShader)
			m_pixelShader->Release();
		if (m_vertexShader)
			m_vertexShader->Release();
		if (m_computeShader)
			m_computeShader->Release();
		if (m_vertexShaderBuffer)
			m_vertexShaderBuffer->Release();
		if (m_pixelShaderBuffer)
			m_pixelShaderBuffer->Release();
		if (m_computeShaderBuffer)
			m_computeShaderBuffer->Release();
	}

private:
	void getUniforms(ID3D10Blob *shader, std::vector<ID3D11Buffer*> &uniformBuffers, ShaderType shaderType)
	{
		ID3D11ShaderReflection* reflector = nullptr;
		D3D_CHECK_RESULT(D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector));

		D3D11_SHADER_DESC shader_desc{};
		D3D_CHECK_RESULT(reflector->GetDesc(&shader_desc));

		for (uint32_t i = 0; i < shader_desc.BoundResources; i++)
		{
			D3D11_SHADER_INPUT_BIND_DESC desc{};
			D3D_CHECK_RESULT(reflector->GetResourceBindingDesc(i, &desc));

			if (desc.Type == D3D_SIT_TEXTURE && desc.Dimension == D3D_SRV_DIMENSION_TEXTURE2D)
			{
				m_uniforms.emplace_back();
				Uniform& uniform = m_uniforms.back();
				uniform.id = UniformID(0);
				uniform.name = desc.Name;
				uniform.shaderType = shaderType;
				uniform.bufferIndex = 0;
				uniform.arrayLength = max(1, desc.BindCount);
				uniform.type = UniformType::Texture2D;
			}
			else if (desc.Type == D3D_SIT_SAMPLER)
			{
				m_uniforms.emplace_back();
				Uniform& uniform = m_uniforms.back();
				uniform.id = UniformID(0);
				uniform.name = desc.Name;
				uniform.shaderType = shaderType;
				uniform.bufferIndex = 0;
				uniform.arrayLength = max(1, desc.BindCount);
				uniform.type = UniformType::Texture2D;
			}
		}

		for (uint32_t i = 0; i < shader_desc.ConstantBuffers; i++)
		{
			D3D11_SHADER_BUFFER_DESC desc {};
			ID3D11ShaderReflectionConstantBuffer *cb = reflector->GetConstantBufferByIndex(i);
			D3D_CHECK_RESULT(cb->GetDesc(&desc));

			D3D11_BUFFER_DESC bufferDesc {};
			bufferDesc.ByteWidth = desc.Size;
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			ID3D11Buffer* buffer;
			D3D_CHECK_RESULT(ctx.device->CreateBuffer(&bufferDesc, nullptr, &buffer));
			uniformBuffers.push_back(buffer);
			
			// get the uniforms
			for (int j = 0; j < desc.Variables; j++)
			{
				D3D11_SHADER_VARIABLE_DESC varDesc{};
				ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(j);
				D3D_CHECK_RESULT(var->GetDesc(&varDesc));

				D3D11_SHADER_TYPE_DESC typeDesc{};
				ID3D11ShaderReflectionType* type = var->GetType();
				D3D_CHECK_RESULT(type->GetDesc(&typeDesc));

				m_uniforms.emplace_back();
				Uniform& uniform = m_uniforms.back();
				uniform.id = UniformID(0);
				uniform.name = varDesc.Name;
				uniform.shaderType = shaderType;
				uniform.bufferIndex = i;
				uniform.arrayLength = max(1, typeDesc.Elements);
				uniform.type = UniformType::None;

				if (typeDesc.Type == D3D_SVT_FLOAT)
				{
					if (typeDesc.Rows == 1)
					{
						if (typeDesc.Columns == 1)
							uniform.type = UniformType::Vec;
						else if (typeDesc.Columns == 2)
							uniform.type = UniformType::Vec2;
						else if (typeDesc.Columns == 3)
							uniform.type = UniformType::Vec3;
						else if (typeDesc.Columns == 4)
							uniform.type = UniformType::Vec4;
					}
					else if (typeDesc.Rows == 4 && typeDesc.Columns == 4)
					{
						uniform.type = UniformType::Mat4;
					}
				}
			}
		}
	}
public:

	void use() override
	{
		ctx.deviceContext->IASetInputLayout(m_layout);

		// Vertex shader
		for (uint32_t iBuffer = 0; iBuffer < m_vertexUniformBuffers.size(); iBuffer++)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource{};
			D3D_CHECK_RESULT(ctx.deviceContext->Map(m_vertexUniformBuffers[iBuffer], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			memcpy(mappedResource.pData, m_vertexUniformValues[iBuffer].data(), sizeof(float) * m_vertexUniformValues[iBuffer].size());
			ctx.deviceContext->Unmap(m_vertexUniformBuffers[iBuffer], 0);
		}
		ctx.deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
		ctx.deviceContext->VSSetConstantBuffers(0, m_vertexUniformBuffers.size(), m_vertexUniformBuffers.data());

		// Pixel shader
		for (uint32_t iBuffer = 0; iBuffer < m_fragmentUniformBuffers.size(); iBuffer++)
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource{};
			D3D_CHECK_RESULT(ctx.deviceContext->Map(m_fragmentUniformBuffers[iBuffer], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
			memcpy(mappedResource.pData, m_fragmentUniformValues[iBuffer].data(), sizeof(float) * m_fragmentUniformValues[iBuffer].size());
			ctx.deviceContext->Unmap(m_fragmentUniformBuffers[iBuffer], 0);
		}
		ctx.deviceContext->PSSetShader(m_pixelShader, nullptr, 0);
		ctx.deviceContext->PSSetConstantBuffers(0, m_fragmentUniformBuffers.size(), m_fragmentUniformBuffers.data());
	}

	void setLayout(VertexData data)
	{
		if (m_layout != nullptr)
			return;
		// Now setup the layout of the data that goes into the shader.
		// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
		std::vector<D3D11_INPUT_ELEMENT_DESC> polygonLayout(data.attributes.size());
		ASSERT(m_attributes.size() == data.attributes.size(), "Incorrect size");
		for (uint32_t i = 0; i < data.attributes.size(); i++)
		{
			polygonLayout[i].SemanticName = m_attributes[i].name.c_str();
			polygonLayout[i].SemanticIndex = m_attributes[i].id.value();
			switch (data.attributes[i].type)
			{
			case VertexFormat::Float: polygonLayout[i].Format = DXGI_FORMAT_R32_FLOAT;  break;
			case VertexFormat::Float2: polygonLayout[i].Format = DXGI_FORMAT_R32G32_FLOAT; break;
			case VertexFormat::Float3: polygonLayout[i].Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
			case VertexFormat::Float4: polygonLayout[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
			case VertexFormat::Byte4: polygonLayout[i].Format = DXGI_FORMAT_R8G8B8A8_SNORM; break;
			case VertexFormat::Ubyte4: polygonLayout[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
			case VertexFormat::Short2: polygonLayout[i].Format = DXGI_FORMAT_R16G16_SNORM; break;
			case VertexFormat::Ushort2: polygonLayout[i].Format = DXGI_FORMAT_R16G16_UNORM; break;
			case VertexFormat::Short4: polygonLayout[i].Format = DXGI_FORMAT_R16G16B16A16_SNORM; break;
			case VertexFormat::Ushort4: polygonLayout[i].Format = DXGI_FORMAT_R16G16B16A16_UNORM; break;
			}
			polygonLayout[i].InputSlot = 0;
			polygonLayout[i].AlignedByteOffset = (i == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT);
			polygonLayout[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			polygonLayout[i].InstanceDataStepRate = 0;
		}

		// Create the vertex input layout.
		D3D_CHECK_RESULT(ctx.device->CreateInputLayout(
			polygonLayout.data(),
			polygonLayout.size(),
			m_vertexShaderBuffer->GetBufferPointer(),
			m_vertexShaderBuffer->GetBufferSize(),
			&m_layout
		));
		m_vertexShaderBuffer->Release();
		m_vertexShaderBuffer = nullptr;
		m_pixelShaderBuffer->Release();
		m_pixelShaderBuffer = nullptr;
		m_computeShaderBuffer->Release();
		m_computeShaderBuffer = nullptr;
	}

	void setFloat1(const char* name, float value) override { throw std::runtime_error("Not supported"); }
	void setFloat2(const char* name, float x, float y) override { throw std::runtime_error("Not supported"); }
	void setFloat3(const char* name, float x, float y, float z) override { throw std::runtime_error("Not supported"); }
	void setFloat4(const char* name, float x, float y, float z, float w) override { throw std::runtime_error("Not supported"); }
	void setUint1 (const char* name, uint32_t value) override { throw std::runtime_error("Not supported"); }
	void setUint2 (const char* name, uint32_t x, uint32_t y) override { throw std::runtime_error("Not supported"); }
	void setUint3 (const char* name, uint32_t x, uint32_t y, uint32_t z) override { throw std::runtime_error("Not supported"); }
	void setUint4 (const char* name, uint32_t x, uint32_t y, uint32_t z, uint32_t w) override { throw std::runtime_error("Not supported"); }
	void setInt1  (const char* name, int32_t value) override { throw std::runtime_error("Not supported"); }
	void setInt2  (const char* name, int32_t x, int32_t y) override { throw std::runtime_error("Not supported"); }
	void setInt3  (const char* name, int32_t x, int32_t y, int32_t z) override { throw std::runtime_error("Not supported"); }
	void setInt4  (const char* name, int32_t x, int32_t y, int32_t z, int32_t w) override { throw std::runtime_error("Not supported"); }
	void setMatrix4(const char* name, const float* data, bool transpose = false) override
	{
		const Uniform* currentUniform = getUniform(name);
		ASSERT(currentUniform->type == UniformType::Mat4, "Incorrect type.");
		ASSERT(transpose == false, "Do not support transpose yet.");
		// Get the offset of the currentUniform in the constant buffer.
		const bool isVertex = (ShaderType)((int)currentUniform->shaderType & (int)ShaderType::Vertex) == ShaderType::Vertex;
		const bool isFrag = (ShaderType)((int)currentUniform->shaderType & (int)ShaderType::Fragment) == ShaderType::Fragment;
		if (isVertex)
		{
			uint32_t offset = 0;
			for (uint32_t iUniform = 0; iUniform < m_uniforms.size(); iUniform++)
			{
				Uniform& uniform = m_uniforms[iUniform];
				if (uniform.type == UniformType::None || uniform.type == UniformType::Texture2D || uniform.type == UniformType::Sampler2D)
					continue;
				uint32_t length = 16 * uniform.arrayLength;
				if (uniform.name == currentUniform->name)
				{
					size_t size = offset + length;
					if (m_vertexUniformValues[currentUniform->bufferIndex].size() < size)
						m_vertexUniformValues[currentUniform->bufferIndex].resize(size);
					memcpy(&m_vertexUniformValues[currentUniform->bufferIndex][offset], data, sizeof(float) * 16);
					break;
				}
				offset += length;
			}
		}
		if (isFrag)
		{
			uint32_t offset = 0;
			for (uint32_t iUniform = 0; iUniform < m_uniforms.size(); iUniform++)
			{
				Uniform& uniform = m_uniforms[iUniform];
				if (uniform.type == UniformType::None || uniform.type == UniformType::Texture2D || uniform.type == UniformType::Sampler2D)
					continue;
				uint32_t length = 16 * uniform.arrayLength;
				if (uniform.name == currentUniform->name)
				{
					size_t size = offset + length;
					if (m_fragmentUniformValues[currentUniform->bufferIndex].size() < size)
						m_fragmentUniformValues[currentUniform->bufferIndex].resize(size);
					memcpy(&m_fragmentUniformValues[currentUniform->bufferIndex][offset], data, sizeof(float) * 16);
					break;
				}
				offset += length;
			}
		}
	}
private:
	ID3D10Blob* m_vertexShaderBuffer;
	ID3D10Blob* m_pixelShaderBuffer;
	ID3D10Blob* m_computeShaderBuffer;
	ID3D11InputLayout* m_layout;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11ComputeShader* m_computeShader;
	std::vector<ID3D11Buffer*> m_vertexUniformBuffers;
	std::vector<ID3D11Buffer*> m_fragmentUniformBuffers;
	std::vector<std::vector<float>> m_vertexUniformValues;
	std::vector< std::vector<float>> m_fragmentUniformValues;
};

D3D11Renderer::D3D11Renderer(Window& window, uint32_t width, uint32_t height)
{
	Device device = getDevice(0);
	Logger::info("Device : ", device.name, " - ", device.memory);

	// --- SwapChain ---
	// Initialize the swap chain description.
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;
	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	// Set the refresh rate of the back buffer.
	swapChain.vsync = true;
	if (swapChain.vsync)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = device.monitors[0].numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = device.monitors[0].denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = glfwGetWin32Window(window.handle());
	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	// Set to full screen or windowed mode.
	swapChainDesc.Windowed = !swapChain.fullscreen;
	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;
	// Set the feature level to DirectX 11.
	D3D_FEATURE_LEVEL featureLevel;
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	D3D_CHECK_RESULT(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		&featureLevel, 1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain.swapChain,
		&ctx.device,
		nullptr,
		&ctx.deviceContext
	));

	// --- Backbuffer
	// Get the pointer to the back buffer.
	ID3D11Texture2D* backBufferPtr;
	D3D_CHECK_RESULT(swapChain.swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr));
	// Create the render target view with the back buffer pointer.
	D3D_CHECK_RESULT(ctx.device->CreateRenderTargetView(backBufferPtr, nullptr, &swapChain.renderTargetView));
	// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

	// Initialize the description of the depth buffer.
	D3D11_TEXTURE2D_DESC depthBufferDesc{};
	// Set up the description of the depth buffer.
	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	// Create the texture for the depth buffer using the filled out description.
	D3D_CHECK_RESULT(ctx.device->CreateTexture2D(&depthBufferDesc, nullptr, &swapChain.depthStencilBuffer));

	// Initialize the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create the depth stencil state.
	D3D_CHECK_RESULT(ctx.device->CreateDepthStencilState(&depthStencilDesc, &swapChain.depthStencilState));

	// Set the depth stencil state.
	ctx.deviceContext->OMSetDepthStencilState(swapChain.depthStencilState, 1);

	// Initailze the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	// Create the depth stencil view.
	D3D_CHECK_RESULT(ctx.device->CreateDepthStencilView(swapChain.depthStencilBuffer, &depthStencilViewDesc, &swapChain.depthStencilView));

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	ctx.deviceContext->OMSetRenderTargets(1, &swapChain.renderTargetView, swapChain.depthStencilView);

	// --- Rasterizer ---
	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	D3D_CHECK_RESULT(ctx.device->CreateRasterizerState(&rasterDesc, &swapChain.rasterState));

	// Now set the rasterizer state.
	ctx.deviceContext->RSSetState(swapChain.rasterState);

	m_backbuffer = std::make_shared<D3D11BackBuffer>(width, height);
}

D3D11Renderer::~D3D11Renderer()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (swapChain.swapChain)
	{
		swapChain.swapChain->SetFullscreenState(false, nullptr);
	}

	if (swapChain.rasterState)
	{
		swapChain.rasterState->Release();
		swapChain.rasterState = 0;
	}

	if (swapChain.depthStencilView)
	{
		swapChain.depthStencilView->Release();
		swapChain.depthStencilView = 0;
	}

	if (swapChain.depthStencilState)
	{
		swapChain.depthStencilState->Release();
		swapChain.depthStencilState = 0;
	}

	if (swapChain.depthStencilBuffer)
	{
		swapChain.depthStencilBuffer->Release();
		swapChain.depthStencilBuffer = 0;
	}

	if (swapChain.renderTargetView)
	{
		swapChain.renderTargetView->Release();
		swapChain.renderTargetView = 0;
	}

	if (ctx.deviceContext)
	{
		ctx.deviceContext->Release();
		ctx.deviceContext = 0;
	}

	if (ctx.device)
	{
		ctx.device->Release();
		ctx.device = 0;
	}

	if (swapChain.swapChain)
	{
		swapChain.swapChain->Release();
		swapChain.swapChain = 0;
	}
}

void D3D11Renderer::resize(uint32_t width, uint32_t height)
{
	m_backbuffer->resize(width, height);
}

void D3D11Renderer::frame()
{
}

void D3D11Renderer::present()
{
	// Present the back buffer to the screen since rendering is complete.
	if (swapChain.vsync)
	{
		// Lock to screen refresh rate.
		swapChain.swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		swapChain.swapChain->Present(0, 0);
	}
}

void D3D11Renderer::viewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// Create the viewport.
	ctx.deviceContext->RSSetViewports(1, &viewport);
}

Rect D3D11Renderer::viewport()
{
	return Rect{};
}

Framebuffer::Ptr D3D11Renderer::backbuffer()
{
	return m_backbuffer;
}

void D3D11Renderer::render(RenderPass& renderPass)
{
	/*{
		// Set framebuffer
		if (framebuffer != nullptr)
		{
			framebuffer->bind(Framebuffer::Type::Both);
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}*/

	/*{
		// Blending
		if (blend == BlendMode::None)
		{
			glDisable(GL_BLEND);
		}
		else
		{
			glEnable(GL_BLEND);
			switch (blend)
			{
			case BlendMode::Zero:
				glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
				break;
			case BlendMode::One:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				break;
			case BlendMode::SrcColor:
				glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);
				break;
			case BlendMode::OneMinusSrcColor:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
				break;
			case BlendMode::DstColor:
				glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);
				break;
			case BlendMode::OneMinusDstColor:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_COLOR);
				break;
			case BlendMode::SrcAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA);
				break;
			case BlendMode::OneMinusSrcAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BlendMode::DstAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				break;
			case BlendMode::OneMinusDstAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
				break;
			case BlendMode::ConstantColor:
				glBlendFunc(GL_SRC_ALPHA, GL_CONSTANT_COLOR);
				break;
			case BlendMode::OneMinusConstantColor:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_CONSTANT_COLOR);
				break;
			case BlendMode::ConstantAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_CONSTANT_ALPHA);
				break;
			case BlendMode::OneMinusConstantAlpha:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
				break;
			case BlendMode::SrcAlphaSaturate:
				glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA_SATURATE);
				break;
			case BlendMode::Src1Color:
				glBlendFunc(GL_SRC_ALPHA, GL_SRC1_COLOR);
				break;
			case BlendMode::OneMinusSrc1Color:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_COLOR);
				break;
			case BlendMode::Src1Alpha:
				glBlendFunc(GL_SRC_ALPHA, GL_SRC1_ALPHA);
				break;
			case BlendMode::OneMinusSrc1Alpha:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_ALPHA);
				break;
			}
		}
	}*/
	/*{
		// Cull
		if (cull == CullMode::None)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			switch (cull)
			{
			case CullMode::FrontFace:
				glCullFace(GL_FRONT);
				glFrontFace(GL_CCW);
				break;
			case CullMode::BackFace:
				glCullFace(GL_BACK);
				glFrontFace(GL_CCW);
				break;
			case CullMode::AllFace:
				glCullFace(GL_FRONT_AND_BACK);
				glFrontFace(GL_CCW);
				break;
			}
		}
	}*/

	/*{
		// TODO set depth
	}*/

	{
		// Viewport
		D3D11_VIEWPORT viewport;
		viewport.Width = (float)renderPass.viewport.w;
		viewport.Height = (float)renderPass.viewport.h;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = renderPass.viewport.x;
		viewport.TopLeftY = renderPass.viewport.y;

		// Create the viewport.
		ctx.deviceContext->RSSetViewports(1, &viewport);
	}

	{
		// TODO Scissor
	}

	{
		// Shader
		if (renderPass.shader == nullptr)
		{
			Logger::error("No shader set for render pass");
			return;
		}
		else
		{
			((D3D11Shader*)renderPass.shader.get())->setLayout(renderPass.mesh->getVertexData());
			renderPass.shader->use();
		}
	}
	{
		// Mesh
		if (renderPass.mesh == nullptr)
		{
			Logger::error("No mesh set for render pass");
			return;
		}
		else
		{
			renderPass.mesh->draw(renderPass.indexCount, renderPass.indexOffset);
		}
	}
}

D3D11Context& D3D11Renderer::context()
{
	return ctx;
}

Device D3D11Renderer::getDevice(uint32_t id)
{
	Device device{};

	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput; // monitor
	// Create a DirectX graphics interface factory.
	D3D_CHECK_RESULT(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));
	// Use the factory to create an adapter for the primary graphics interface (video card).
	D3D_CHECK_RESULT(factory->EnumAdapters(id, &adapter));
	// Enumerate the primary adapter output (monitor).
	D3D_CHECK_RESULT(adapter->EnumOutputs(id, &adapterOutput));

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	uint32_t numModes = 0;
	D3D_CHECK_RESULT(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr));

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	std::vector<DXGI_MODE_DESC> displayModeList(numModes);

	// Now fill the display mode list structures.
	D3D_CHECK_RESULT(adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.data()));

	// Now go through all the display modes
	device.monitors.resize(numModes);
	for (unsigned int i = 0; i < numModes; i++)
	{
		Monitor& monitor = device.monitors[i];
		monitor.width = displayModeList[i].Width;
		monitor.height = displayModeList[i].Height;
		monitor.numerator = displayModeList[i].RefreshRate.Numerator;
		monitor.denominator = displayModeList[i].RefreshRate.Denominator;
	}
	// Get the adapter (video card) description.
	DXGI_ADAPTER_DESC adapterDesc;
	D3D_CHECK_RESULT(adapter->GetDesc(&adapterDesc));

	// Store the dedicated video card memory in megabytes.
	device.memory = (uint32_t)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	size_t stringLength;
	int error = wcstombs_s(&stringLength, device.name, 128, adapterDesc.Description, 128);
	if (error != 0)
		Logger::error("Could not get video card name");

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

	// Release the adapter.
	adapter->Release();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = 0;

	return device;
}

uint32_t D3D11Renderer::deviceCount()
{
	return 0;
}

Texture::Ptr D3D11Renderer::createTexture(uint32_t width, uint32_t height, Texture::Format format, const uint8_t* data, Sampler::Filter filter)
{
	return std::make_shared<D3D11Texture>(width, height, format, data, filter);
}

Framebuffer::Ptr D3D11Renderer::createFramebuffer(uint32_t width, uint32_t height, Framebuffer::Attachment* attachment, size_t count)
{
	return std::make_shared<D3D11Framebuffer>(width, height, attachment, count);
}

Mesh::Ptr D3D11Renderer::createMesh()
{
	return std::make_shared<D3D11Mesh>();
}

ShaderID D3D11Renderer::compile(const char* content, ShaderType type)
{
	ID3DBlob* shaderBuffer = nullptr;
	ID3DBlob* errorMessage = nullptr;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
	// Compile from command line instead
	HRESULT result = D3DCompile(
		content,
		strlen(content),
		nullptr, // error string
		nullptr,
		nullptr,
		"vs_main",
		"vs_5_0",
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
			return ShaderID(0);
		}
		else
		{
			Logger::error("[compilation] Missing shader file");
			return ShaderID(0);
		}
	}
	return ShaderID((uintptr_t)shaderBuffer);
}

Shader::Ptr D3D11Renderer::createShader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes)
{
	return std::make_shared<D3D11Shader>(vert, frag, compute, attributes);
}


};
#endif