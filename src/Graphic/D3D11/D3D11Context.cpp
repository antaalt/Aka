#if defined(AKA_USE_D3D11)

#include "D3D11Context.h"
#include "D3D11Device.h"

#include <Aka/OS/Logger.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace aka {

D3D11Context::D3D11Context(D3D11Device* device) :
	m_device(device),
	m_debugInfoQueue(nullptr)
{
#if defined(DEBUG)
	D3D_CHECK_RESULT(m_device->device()->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&m_debugInfoQueue));
	D3D_CHECK_RESULT(m_debugInfoQueue->PushEmptyStorageFilter());
#endif
}

D3D11Context::~D3D11Context()
{
	if (m_debugInfoQueue)
		m_debugInfoQueue->Release();
	for (D3D11Sampler& pass : m_samplerCache)
		pass.samplerState->Release();
	for (D3D11Depth& pass : m_depthCache)
		pass.depthState->Release();
	for (D3D11Blend& pass : m_blendCache)
		pass.blendState->Release();
	for (D3D11RasterPass& pass : m_rasterCache)
		pass.rasterState->Release();
}

void D3D11Context::log()
{
	UINT64 messageCount = m_debugInfoQueue->GetNumStoredMessages();
	for (UINT64 i = 0; i < messageCount; i++) {
		SIZE_T messageSize = 0;
		HRESULT res = m_debugInfoQueue->GetMessage(i, nullptr, &messageSize);
		if (SUCCEEDED(res) || messageSize > 0u)
		{
			D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc(messageSize); // Allocate enough space
			res = m_debugInfoQueue->GetMessage(i, message, &messageSize); // Get the actual message
			if (SUCCEEDED(res))
			{
				switch (message->Severity)
				{
				case D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_CORRUPTION:
					Logger::critical("[D3D11] ", message->pDescription);
					break;
				case D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_ERROR:
					Logger::error("[D3D11] ", message->pDescription);
					break;
				case D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_WARNING:
					Logger::warn("[D3D11] ", message->pDescription);
					break;
				default:
				case D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_MESSAGE:
				case D3D11_MESSAGE_SEVERITY::D3D11_MESSAGE_SEVERITY_INFO:
					Logger::debug("[D3D11] ", message->pDescription);
					break;
				}
			}
			free(message);
		}
	}
	m_debugInfoQueue->ClearStoredMessages();
}

UINT8 blendMask(BlendMask mask)
{
	UINT8 m = 0;
	if ((BlendMask)((int)mask & (int)BlendMask::Red) == BlendMask::Red)
		m |= D3D11_COLOR_WRITE_ENABLE_RED;
	if ((BlendMask)((int)mask & (int)BlendMask::Green) == BlendMask::Green)
		m |= D3D11_COLOR_WRITE_ENABLE_GREEN;
	if ((BlendMask)((int)mask & (int)BlendMask::Blue) == BlendMask::Blue)
		m |= D3D11_COLOR_WRITE_ENABLE_BLUE;
	if ((BlendMask)((int)mask & (int)BlendMask::Alpha) == BlendMask::Alpha)
		m |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
	return m;
}
D3D11_BLEND_OP blendOp(BlendOp op)
{
	switch (op)
	{
	case BlendOp::Add: return D3D11_BLEND_OP_ADD;
	case BlendOp::Substract: return D3D11_BLEND_OP_SUBTRACT;
	case BlendOp::ReverseSubstract: return D3D11_BLEND_OP_REV_SUBTRACT;
	case BlendOp::Min: return D3D11_BLEND_OP_MIN;
	case BlendOp::Max: return D3D11_BLEND_OP_MAX;
	}
	return D3D11_BLEND_OP_ADD;
}
D3D11_BLEND blendFactor(BlendMode mode)
{
	switch (mode)
	{
	case BlendMode::Zero: return D3D11_BLEND_ZERO;
	case BlendMode::One: return D3D11_BLEND_ONE;
	case BlendMode::SrcColor: return D3D11_BLEND_SRC_COLOR;
	case BlendMode::OneMinusSrcColor: return D3D11_BLEND_INV_SRC_COLOR;
	case BlendMode::DstColor: return D3D11_BLEND_DEST_COLOR;
	case BlendMode::OneMinusDstColor: return D3D11_BLEND_INV_DEST_COLOR;
	case BlendMode::SrcAlpha: return D3D11_BLEND_SRC_ALPHA;
	case BlendMode::OneMinusSrcAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
	case BlendMode::DstAlpha: return D3D11_BLEND_DEST_ALPHA;
	case BlendMode::OneMinusDstAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
	case BlendMode::ConstantColor: return D3D11_BLEND_BLEND_FACTOR;
	case BlendMode::OneMinusConstantColor: return D3D11_BLEND_INV_BLEND_FACTOR;
	case BlendMode::ConstantAlpha: return D3D11_BLEND_BLEND_FACTOR;
	case BlendMode::OneMinusConstantAlpha: return D3D11_BLEND_INV_BLEND_FACTOR;
	case BlendMode::SrcAlphaSaturate: return D3D11_BLEND_SRC_ALPHA_SAT;
	case BlendMode::Src1Color: return D3D11_BLEND_SRC1_COLOR;
	case BlendMode::OneMinusSrc1Color: return D3D11_BLEND_INV_SRC1_COLOR;
	case BlendMode::Src1Alpha: return D3D11_BLEND_SRC1_ALPHA;
	case BlendMode::OneMinusSrc1Alpha: return D3D11_BLEND_INV_SRC1_ALPHA;
	}
	return D3D11_BLEND_ZERO;
}
ID3D11BlendState* D3D11Context::getBlendingState(const Blending& blending)
{
	for (D3D11Blend& blend : m_blendCache)
		if (blend.blend == blending)
			return blend.blendState;

	D3D11_BLEND_DESC desc{};
	desc.AlphaToCoverageEnable = 0;
	desc.IndependentBlendEnable = 0;

	desc.RenderTarget[0].BlendEnable = blending.enabled();
	desc.RenderTarget[0].RenderTargetWriteMask = blendMask(blending.mask);
	if (blending.enabled())
	{
		desc.RenderTarget[0].BlendOp = blendOp(blending.colorOp);
		desc.RenderTarget[0].SrcBlend = blendFactor(blending.colorModeSrc);
		desc.RenderTarget[0].DestBlend = blendFactor(blending.colorModeDst);
		desc.RenderTarget[0].BlendOpAlpha = blendOp(blending.alphaOp);
		desc.RenderTarget[0].SrcBlendAlpha = blendFactor(blending.alphaModeSrc);
		desc.RenderTarget[0].DestBlendAlpha = blendFactor(blending.alphaModeDst);
	}

	for (uint32_t i = 1; i < 8; i++)
		desc.RenderTarget[i] = desc.RenderTarget[0];

	ID3D11BlendState* result = nullptr;
	HRESULT res = m_device->device()->CreateBlendState(&desc, &result);
	if (SUCCEEDED(res))
	{
		D3D11Blend blend;
		blend.blend = blending;
		blend.blendState = result;
		m_blendCache.push_back(blend);
		return m_blendCache.back().blendState;
	}
	return nullptr;
}

D3D11_STENCIL_OP stencilMode(StencilMode op)
{
	switch (op)
	{
	default:
	case StencilMode::Keep: return D3D11_STENCIL_OP_KEEP;
	case StencilMode::Zero: return D3D11_STENCIL_OP_ZERO;
	case StencilMode::Replace: return D3D11_STENCIL_OP_REPLACE;
	case StencilMode::Increment: return D3D11_STENCIL_OP_INCR_SAT;
	case StencilMode::IncrementWrap: return D3D11_STENCIL_OP_INCR;
	case StencilMode::Decrement: return D3D11_STENCIL_OP_DECR_SAT;
	case StencilMode::DecrementWrap: return D3D11_STENCIL_OP_DECR;
	case StencilMode::Invert: return D3D11_STENCIL_OP_INVERT;
	}
}
D3D11_COMPARISON_FUNC stencilCompare(StencilCompare func)
{
	switch (func)
	{
	default:
	case StencilCompare::None: return D3D11_COMPARISON_NEVER;
	case StencilCompare::Never: return D3D11_COMPARISON_NEVER;
	case StencilCompare::Less: return D3D11_COMPARISON_LESS;
	case StencilCompare::LessOrEqual: return D3D11_COMPARISON_LESS_EQUAL;
	case StencilCompare::Greater: return D3D11_COMPARISON_GREATER;
	case StencilCompare::GreaterOrEqual: return D3D11_COMPARISON_GREATER_EQUAL;
	case StencilCompare::Equal: return D3D11_COMPARISON_EQUAL;
	case StencilCompare::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
	case StencilCompare::Always: return D3D11_COMPARISON_ALWAYS;
	}
}
D3D11_COMPARISON_FUNC depthFunc(DepthCompare func)
{
	switch (func)
	{
	default:
	case DepthCompare::None: return D3D11_COMPARISON_NEVER;
	case DepthCompare::Never: return D3D11_COMPARISON_NEVER;
	case DepthCompare::Less: return D3D11_COMPARISON_LESS;
	case DepthCompare::LessOrEqual: return D3D11_COMPARISON_LESS_EQUAL;
	case DepthCompare::Greater: return D3D11_COMPARISON_GREATER;
	case DepthCompare::GreaterOrEqual: return D3D11_COMPARISON_GREATER_EQUAL;
	case DepthCompare::Equal: return D3D11_COMPARISON_EQUAL;
	case DepthCompare::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
	case DepthCompare::Always: return D3D11_COMPARISON_ALWAYS;
	}
}
ID3D11DepthStencilState* D3D11Context::getDepthState(const Depth& depth, const Stencil& stencil)
{
	for (D3D11Depth& d : m_depthCache)
		if (d.depth == depth && d.stencil == stencil)
			return d.depthState;

	D3D11_DEPTH_STENCIL_DESC desc{};

	desc.DepthEnable = (depth.compare != DepthCompare::None);
	desc.DepthWriteMask = depth.mask ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = depthFunc(depth.compare);

	desc.StencilEnable = stencil.enabled();
	desc.StencilReadMask = stencil.readMask;
	desc.StencilWriteMask = stencil.writeMask;
	desc.FrontFace.StencilFailOp = stencilMode(stencil.front.stencilFailed);
	desc.FrontFace.StencilDepthFailOp = stencilMode(stencil.front.stencilDepthFailed);
	desc.FrontFace.StencilPassOp = stencilMode(stencil.front.stencilPassed);
	desc.FrontFace.StencilFunc = stencilCompare(stencil.front.mode);
	desc.BackFace.StencilFailOp = stencilMode(stencil.back.stencilFailed);
	desc.BackFace.StencilDepthFailOp = stencilMode(stencil.back.stencilDepthFailed);
	desc.BackFace.StencilPassOp = stencilMode(stencil.back.stencilPassed);
	desc.BackFace.StencilFunc = stencilCompare(stencil.back.mode);

	ID3D11DepthStencilState* result;
	HRESULT res = m_device->device()->CreateDepthStencilState(&desc, &result);
	if (SUCCEEDED(res))
	{
		D3D11Depth d;
		d.depth = depth;
		d.depthState = result;
		m_depthCache.push_back(d);
		return m_depthCache.back().depthState;
	}
	return nullptr;
}
ID3D11RasterizerState* D3D11Context::getRasterizerState(const Culling& culling)
{
	for (D3D11RasterPass& pass : m_rasterCache)
		if (pass.cull == culling)
			return pass.rasterState;
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	switch (culling.mode)
	{
	case CullMode::None: rasterDesc.CullMode = D3D11_CULL_NONE; break;
	case CullMode::FrontFace: rasterDesc.CullMode = D3D11_CULL_FRONT; break;
	case CullMode::BackFace: rasterDesc.CullMode = D3D11_CULL_BACK; break;
	}
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = (culling.order == CullOrder::CounterClockWise);
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	ID3D11RasterizerState* rasterState;
	HRESULT res = m_device->device()->CreateRasterizerState(&rasterDesc, &rasterState);
	if (SUCCEEDED(res))
	{
		D3D11RasterPass pass;
		pass.cull = culling;
		pass.rasterState = rasterState;
		m_rasterCache.push_back(pass);
		return m_rasterCache.back().rasterState;
	}
	return nullptr;
}
ID3D11SamplerState* D3D11Context::getSamplerState(const TextureSampler& sampler)
{
	for (D3D11Sampler& s : m_samplerCache)
		if (s.sampler == sampler)
			return s.samplerState;

	D3D11_SAMPLER_DESC desc{};

	switch (sampler.wrapU)
	{
	case TextureWrap::Repeat: desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; break;
	case TextureWrap::ClampToEdge: desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP; break;
	case TextureWrap::ClampToBorder: desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER; break;
	case TextureWrap::Mirror: desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR; break;
	}
	switch (sampler.wrapV)
	{
	case TextureWrap::Repeat: desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; break;
	case TextureWrap::ClampToEdge: desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP; break;
	case TextureWrap::ClampToBorder: desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER; break;
	case TextureWrap::Mirror: desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR; break;
	}
	switch (sampler.wrapW)
	{
	case TextureWrap::Repeat: desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; break;
	case TextureWrap::ClampToEdge: desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP; break;
	case TextureWrap::ClampToBorder: desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER; break;
	case TextureWrap::Mirror: desc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR; break;
	}
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	TextureMipMapMode mipmapMode = (sampler.mipmapMode == TextureMipMapMode::None) ? TextureMipMapMode::Nearest : sampler.mipmapMode;
	if (sampler.filterMin == sampler.filterMag)
	{
		if (sampler.filterMag == TextureFilter::Nearest && mipmapMode == TextureMipMapMode::Nearest)
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		else if (sampler.filterMag == TextureFilter::Nearest && mipmapMode == TextureMipMapMode::Linear)
			desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		else if (sampler.filterMag == TextureFilter::Linear && mipmapMode == TextureMipMapMode::Nearest)
			desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		else if (sampler.filterMag == TextureFilter::Linear && mipmapMode == TextureMipMapMode::Linear)
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		else
			Logger::error("Invalid values for texture filter min");
	}
	else if (sampler.filterMin == TextureFilter::Nearest)
	{
		if (sampler.filterMag == TextureFilter::Linear && mipmapMode == TextureMipMapMode::Nearest)
			desc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		else if (sampler.filterMag == TextureFilter::Linear && mipmapMode == TextureMipMapMode::Linear)
			desc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		else
			Logger::error("Invalid values for texture filter min");
	}
	else if (sampler.filterMin == TextureFilter::Linear)
	{
		if (sampler.filterMag == TextureFilter::Nearest && mipmapMode == TextureMipMapMode::Nearest)
			desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		else if (sampler.filterMag == TextureFilter::Nearest && mipmapMode == TextureMipMapMode::Linear)
			desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		else
			Logger::error("Invalid values for texture filter min");
	}
	else
	{
		Logger::error("Invalid values for texture filter min");
	}

	ID3D11SamplerState* result;
	HRESULT res = m_device->device()->CreateSamplerState(&desc, &result);
	if (SUCCEEDED(res))
	{
		D3D11Sampler s;
		s.sampler = sampler;
		s.samplerState = result;
		m_samplerCache.push_back(s);
		return m_samplerCache.back().samplerState;
	}
	return nullptr;
}

};

#endif
