#if defined(AKA_USE_D3D11)
#include "D3D11Material.h"

#include "D3D11Context.h"
#include "D3D11Program.h"
#include "D3D11Device.h"

#include <Aka/OS/Logger.h>

namespace aka {

D3D11Material::D3D11Material(D3D11Device* device, Program::Ptr shader) :
	Material(shader),
	m_device(device)
{
	uint32_t textureCount = 0;
	uint32_t bufferCount = 0;
	for (const Uniform& uniform : *m_program)
	{
		switch (uniform.type)
		{
		case UniformType::Buffer:
			bufferCount = max(bufferCount, uniform.binding + uniform.count);
			break;
		case UniformType::TextureCubemap:
		case UniformType::Texture2D:
		case UniformType::Texture2DMultisample:
			textureCount = max(textureCount, uniform.binding + uniform.count);
			break;
		case UniformType::Float:
		case UniformType::Int:
		case UniformType::UnsignedInt:
		case UniformType::Vec2:
		case UniformType::Vec3:
		case UniformType::Vec4:
		case UniformType::Mat3:
		case UniformType::Mat4:
		default:
			break;
		}
	}
	m_buffers.resize(bufferCount, nullptr);
	m_textures.resize(textureCount, nullptr);
	m_samplers.resize(textureCount, TextureSampler::nearest);
}

D3D11Material::~D3D11Material()
{
}

void D3D11Material::use() const
{
	D3D11Program* d3dProgram = reinterpret_cast<D3D11Program*>(m_program.get());
	d3dProgram->use();
	uint32_t textureUnit = 0;
	uint32_t bufferUnit = 0;
	// Alignement hlsl :
	// Close to std140 GLSL
	// https://docs.microsoft.com/fr-fr/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules?redirectedfrom=MSDN
	static const size_t shaderTypeCount = 4;
	std::vector<ID3D11Buffer*> uniformBuffers[shaderTypeCount];
	std::vector<ID3D11ShaderResourceView*> shaderResourceViews[shaderTypeCount];
	std::vector<ID3D11SamplerState*> samplerStates[shaderTypeCount];
	for (const Uniform& uniform : *m_program)
	{
		const bool isShaderType[shaderTypeCount] = {
			(ShaderType)((int)uniform.shaderType & (int)ShaderType::Vertex) == ShaderType::Vertex,
			(ShaderType)((int)uniform.shaderType & (int)ShaderType::Fragment) == ShaderType::Fragment,
			(ShaderType)((int)uniform.shaderType & (int)ShaderType::Geometry) == ShaderType::Geometry,
			(ShaderType)((int)uniform.shaderType & (int)ShaderType::Compute) == ShaderType::Compute
		};
		switch (uniform.type)
		{
		case UniformType::Buffer: {
			for (uint32_t i = 0; i < uniform.count; i++)
			{
				Buffer::Ptr buffer = m_buffers[uniform.binding];
				if (buffer != nullptr)
				{
					for (size_t i = 0; i < shaderTypeCount; i++)
						if (isShaderType[i])
							uniformBuffers[i].push_back((ID3D11Buffer*)buffer->handle().value());
				}
				else
				{
					for (size_t i = 0; i < shaderTypeCount; i++)
						if (isShaderType[i])
							uniformBuffers[i].push_back(nullptr);
				}
			}
			break;
		}
		case UniformType::Texture2D:
		case UniformType::Texture2DMultisample:
		case UniformType::TextureCubemap: {
			for (uint32_t i = 0; i < uniform.count; i++)
			{
				Texture::Ptr texture = m_textures[uniform.binding];
				TextureSampler sampler = m_samplers[uniform.binding];
				if (texture != nullptr)
				{
					for (size_t i = 0; i < shaderTypeCount; i++)
					{
						if (isShaderType[i])
						{
							shaderResourceViews[i].push_back((ID3D11ShaderResourceView*)texture->handle().value());
							samplerStates[i].push_back(m_device->ctx()->getSamplerState(sampler));
						}
					}
				}
				else
				{
					for (size_t i = 0; i < shaderTypeCount; i++)
					{
						if (isShaderType[i])
						{
							shaderResourceViews[i].push_back(nullptr);
							samplerStates[i].push_back(nullptr);
						}
					}
				}
			}
			break;
		}
		default:
			break;
		}
	}
	// Fill textures units from data
	if (shaderResourceViews[0].size() > 0)
	{
		AKA_ASSERT(shaderResourceViews[0].size() == samplerStates[0].size(), "Invalid count.");
		m_device->context()->VSSetShaderResources(0, (UINT)shaderResourceViews[0].size(), shaderResourceViews[0].data());
		m_device->context()->VSSetSamplers(0, (UINT)samplerStates[0].size(), samplerStates[0].data());
	}
	if (shaderResourceViews[1].size() > 0)
	{
		AKA_ASSERT(shaderResourceViews[1].size() == samplerStates[1].size(), "Invalid count.");
		m_device->context()->PSSetShaderResources(0, (UINT)shaderResourceViews[1].size(), shaderResourceViews[1].data());
		m_device->context()->PSSetSamplers(0, (UINT)samplerStates[1].size(), samplerStates[1].data());
	}
	if (shaderResourceViews[2].size() > 0)
	{
		AKA_ASSERT(shaderResourceViews[2].size() == samplerStates[2].size(), "Invalid count.");
		m_device->context()->GSSetShaderResources(0, (UINT)shaderResourceViews[2].size(), shaderResourceViews[2].data());
		m_device->context()->GSSetSamplers(0, (UINT)samplerStates[2].size(), samplerStates[2].data());
	}
	if (shaderResourceViews[3].size() > 0)
	{
		AKA_ASSERT(shaderResourceViews[3].size() == samplerStates[3].size(), "Invalid count.");
		m_device->context()->CSSetShaderResources(0, (UINT)shaderResourceViews[3].size(), shaderResourceViews[3].data());
		m_device->context()->CSSetSamplers(0, (UINT)samplerStates[3].size(), samplerStates[3].data());
	}
	// Fill buffer slot from buffers
	if (uniformBuffers[0].size() > 0)
		m_device->context()->VSSetConstantBuffers(0, (UINT)uniformBuffers[0].size(), uniformBuffers[0].data());
	if (uniformBuffers[1].size() > 0)
		m_device->context()->PSSetConstantBuffers(0, (UINT)uniformBuffers[1].size(), uniformBuffers[1].data());
	if (uniformBuffers[2].size() > 0)
		m_device->context()->GSSetConstantBuffers(0, (UINT)uniformBuffers[2].size(), uniformBuffers[2].data());
	if (uniformBuffers[3].size() > 0)
		m_device->context()->GSSetConstantBuffers(0, (UINT)uniformBuffers[3].size(), uniformBuffers[3].data());
}

};

#endif