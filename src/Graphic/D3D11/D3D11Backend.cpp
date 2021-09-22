#if defined(AKA_USE_D3D11)
#include <Aka/Graphic/GraphicBackend.h>
#include <Aka/Drawing/Renderer2D.h>
#include <Aka/Core/Debug.h>
#include <Aka/Core/Event.h>
#include <Aka/OS/Logger.h>
#include <Aka/Platform/Platform.h>
#include <Aka/Platform/PlatformBackend.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <stdexcept>
#include <array>
#include <map>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

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
            AKA_STRINGIFY(result),  \
            __FILE__,               \
            __LINE__				\
        );							\
        ::aka::Logger::error(buffer);\
		dctx.log();                 \
		AKA_DEBUG_BREAK;            \
	}								\
}

namespace aka {

class D3D11BackBuffer;

struct D3D11Context
{
	struct D3D11SwapChain {
		IDXGISwapChain* swapChain = nullptr;
		bool vsync = true;
		bool fullscreen = false;
	} swapchain;

#if defined(DEBUG)
	ID3D11InfoQueue* debugInfoQueue = nullptr;
#endif
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;

	GraphicDeviceFeatures features = {};

	void log()
	{
		UINT64 messageCount = debugInfoQueue->GetNumStoredMessages();
		for (UINT64 i = 0; i < messageCount; i++) {
			SIZE_T messageSize = 0;
			HRESULT res = debugInfoQueue->GetMessage(i, nullptr, &messageSize);
			if (SUCCEEDED(res) || messageSize > 0u)
			{
				D3D11_MESSAGE* message = (D3D11_MESSAGE*)malloc(messageSize); // Allocate enough space
				res = debugInfoQueue->GetMessage(i, message, &messageSize); // Get the actual message
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
		debugInfoQueue->ClearStoredMessages();
	}
};

static D3D11Context dctx;

struct D3D11RasterPass {
	Culling cull{};
	ID3D11RasterizerState* rasterState = nullptr;
	static ID3D11RasterizerState* get(Culling cull)
	{
		for (D3D11RasterPass& pass : cache)
			if (pass.cull == cull)
				return pass.rasterState;
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		switch (cull.mode)
		{
		case CullMode::None: rasterDesc.CullMode = D3D11_CULL_NONE; break;
		case CullMode::FrontFace: rasterDesc.CullMode = D3D11_CULL_FRONT; break;
		case CullMode::BackFace: rasterDesc.CullMode = D3D11_CULL_BACK; break;
		}
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = (cull.order == CullOrder::CounterClockWise);
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;

		// Create the rasterizer state from the description we just filled out.
		ID3D11RasterizerState* rasterState;
		HRESULT res = dctx.device->CreateRasterizerState(&rasterDesc, &rasterState);
		if (SUCCEEDED(res))
		{
			D3D11RasterPass pass;
			pass.cull = cull;
			pass.rasterState = rasterState;
			cache.push_back(pass);
			return cache.back().rasterState;
		}
		return nullptr;
	}
	static void clear()
	{
		for (D3D11RasterPass& pass : cache)
			pass.rasterState->Release();
	}
private:
	static std::vector<D3D11RasterPass> cache;
};

struct D3D11Sampler
{
	TextureSampler sampler = {};
	ID3D11SamplerState* samplerState = nullptr;
	static ID3D11SamplerState* get(TextureSampler sampler)
	{
		for (D3D11Sampler& s : cache)
			if (s.sampler == sampler)
				return s.samplerState;

		D3D11_SAMPLER_DESC desc {};
		
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
		HRESULT res = dctx.device->CreateSamplerState(&desc, &result);
		if (SUCCEEDED(res))
		{
			D3D11Sampler s;
			s.sampler = sampler;
			s.samplerState = result;
			cache.push_back(s);
			return cache.back().samplerState;
		}
		return nullptr;
	}
	static void clear()
	{
		for (D3D11Sampler& pass : cache)
			pass.samplerState->Release();
	}
private:
	static std::vector<D3D11Sampler> cache;
};

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

struct D3D11Depth
{
	Depth depth = Depth::none;
	Stencil stencil = Stencil::none;
	ID3D11DepthStencilState* depthState = nullptr;
	static ID3D11DepthStencilState* get(Depth depth, Stencil stencil)
	{
		for (D3D11Depth& d : cache)
			if (d.depth == depth && d.stencil == stencil)
				return d.depthState;

		D3D11_DEPTH_STENCIL_DESC desc {};

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
		HRESULT res = dctx.device->CreateDepthStencilState(&desc, &result);
		if (SUCCEEDED(res))
		{
			D3D11Depth d;
			d.depth = depth;
			d.depthState = result;
			cache.push_back(d);
			return cache.back().depthState;
		}
		return nullptr;
	}
	static void clear()
	{
		for (D3D11Depth& depth : cache)
			depth.depthState->Release();
	}
private:
	static std::vector<D3D11Depth> cache;
};

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

struct D3D11Blend
{
	Blending blend = Blending::none;
	ID3D11BlendState* blendState = nullptr;
	static ID3D11BlendState* get(Blending blending)
	{
		for (D3D11Blend& blend : cache)
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
		HRESULT res = dctx.device->CreateBlendState(&desc, &result);
		if (SUCCEEDED(res))
		{
			D3D11Blend blend;
			blend.blend = blending;
			blend.blendState = result;
			cache.push_back(blend);
			return cache.back().blendState;
		}
		return nullptr;
	}
	static void clear()
	{
		for (D3D11Blend& blend : cache)
			blend.blendState->Release();
	}
private:
	static std::vector<D3D11Blend> cache;
};


std::vector<D3D11RasterPass> D3D11RasterPass::cache;
std::vector<D3D11Sampler> D3D11Sampler::cache;
std::vector<D3D11Depth> D3D11Depth::cache;
std::vector<D3D11Blend> D3D11Blend::cache;

void SetDebugName(ID3D11DeviceChild* child, const std::string& name)
{
	if (child != nullptr)
		child->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
}

DXGI_FORMAT d3dShaderDataFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_D16_UNORM:
		return DXGI_FORMAT_R16_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_TYPELESS;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24G8_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	}
	return format;
}

DXGI_FORMAT d3dViewFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_D16_UNORM:
		return DXGI_FORMAT_R16_UNORM;
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	}
	return format;
}

DXGI_FORMAT d3dformat(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::R8:
	case TextureFormat::R8U: return DXGI_FORMAT_R8_UNORM;
	case TextureFormat::R16:
	case TextureFormat::R16U: return DXGI_FORMAT_R16_UNORM;
	case TextureFormat::R16F: return DXGI_FORMAT_R16_FLOAT;
	case TextureFormat::R32F: return DXGI_FORMAT_R32_FLOAT;

	case TextureFormat::RG8:
	case TextureFormat::RG8U: return DXGI_FORMAT_R8G8_UNORM;
	case TextureFormat::RG16:
	case TextureFormat::RG16U: return DXGI_FORMAT_R16G16_UNORM;
	case TextureFormat::RG16F: return DXGI_FORMAT_R16G16_FLOAT;
	case TextureFormat::RG32F: return DXGI_FORMAT_R32G32_FLOAT;

	case TextureFormat::RGB8:
	case TextureFormat::RGB8U: break; //return DXGI_FORMAT_R8G8B8_UNORM;
	case TextureFormat::RGB16:
	case TextureFormat::RGB16U: break; //return DXGI_FORMAT_R16G16B16_UNORM;
	case TextureFormat::RGB16F: break; //return DXGI_FORMAT_R16G16B16_FLOAT;
	case TextureFormat::RGB32F: return DXGI_FORMAT_R32G32B32_FLOAT;

	case TextureFormat::RGBA8:
	case TextureFormat::RGBA8U: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA16U: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TextureFormat::RGBA16F: return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case TextureFormat::RGBA32F: return DXGI_FORMAT_R32G32B32A32_FLOAT;

	case TextureFormat::Depth: return DXGI_FORMAT_D32_FLOAT;
	case TextureFormat::Depth16: return DXGI_FORMAT_D16_UNORM;
	case TextureFormat::Depth24: break; //return DXGI_FORMAT_D24_UNORM;
	case TextureFormat::Depth32: break; //return DXGI_FORMAT_D32_UNORM;
	case TextureFormat::Depth32F: return DXGI_FORMAT_D32_FLOAT;
	case TextureFormat::DepthStencil: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case TextureFormat::Depth24Stencil8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case TextureFormat::Depth32FStencil8: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	default: break;
	}
	Logger::error("Format not supported : ", (int)format);
	return DXGI_FORMAT_UNKNOWN;
}
uint32_t d3dComponent(TextureFormat format)
{
	switch (format)
	{
	case TextureFormat::R8:
	case TextureFormat::R16:
	case TextureFormat::R8U:
	case TextureFormat::R16U:
	case TextureFormat::R16F:
	case TextureFormat::R32F:
		return 1;

	case TextureFormat::RG8:
	case TextureFormat::RG16:
	case TextureFormat::RG8U:
	case TextureFormat::RG16U:
	case TextureFormat::RG16F:
	case TextureFormat::RG32F:
		return 2;

	case TextureFormat::RGB8:
	case TextureFormat::RGB16:
	case TextureFormat::RGB8U:
	case TextureFormat::RGB16U:
	case TextureFormat::RGB16F:
	case TextureFormat::RGB32F:
		return 3;

	case TextureFormat::RGBA8:
	case TextureFormat::RGBA16:
	case TextureFormat::RGBA8U:
	case TextureFormat::RGBA16U:
	case TextureFormat::RGBA16F:
	case TextureFormat::RGBA32F:
		return 4;

	case TextureFormat::Depth:
	case TextureFormat::Depth16:
	case TextureFormat::Depth24:
	case TextureFormat::Depth32:
	case TextureFormat::Depth32F:
	case TextureFormat::DepthStencil:
	case TextureFormat::Depth0Stencil8:
	case TextureFormat::Depth24Stencil8:
	case TextureFormat::Depth32FStencil8:
		return 1;
	default:
		Logger::error("Format not supported : ", (int)format);
		return 0;
	}
}

class D3D11Material;

struct D3D11Texture
{
	D3D11Texture() :
		m_texture(nullptr),
		m_view(nullptr),
		m_component(0),
		m_d3dFormat(DXGI_FORMAT_UNKNOWN)
	{}
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_view;
	DXGI_FORMAT m_d3dFormat;
	uint32_t m_component;

	static D3D11Texture* convert(const Texture::Ptr& texture);
};

class D3D11Texture2D : public Texture2D, public D3D11Texture
{
public:
	D3D11Texture2D(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		const void* data
	) :
		Texture2D(width, height, format, flags),
		D3D11Texture()
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.BindFlags = 0;

		bool isShaderResource = (TextureFlag::ShaderResource & flags) == TextureFlag::ShaderResource;
		bool isRenderTarget = (TextureFlag::RenderTarget & flags) == TextureFlag::RenderTarget;
		bool hasMips = (TextureFlag::GenerateMips & flags) == TextureFlag::RenderTarget;

		if (hasMips)
		{
			desc.MipLevels = 0;
			desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
		if (isShaderResource)
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		m_d3dFormat = d3dformat(format);
		m_component = d3dComponent(format);

		if (isRenderTarget)
		{
			if (isDepth(format))
				desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
			else
				desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}
		if (isShaderResource)
			desc.Format = d3dShaderDataFormat(m_d3dFormat);
		else
			desc.Format = m_d3dFormat;

		D3D11_SUBRESOURCE_DATA subResources{};
		subResources.pSysMem = data;
		subResources.SysMemPitch = width * size(format);
		subResources.SysMemSlicePitch = 0;
		D3D11_SUBRESOURCE_DATA* sub = nullptr;
		if (data != nullptr)
			sub = &subResources;
		D3D_CHECK_RESULT(dctx.device->CreateTexture2D(&desc, sub, &m_texture));
		if (isShaderResource)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
			viewDesc.Format = d3dViewFormat(m_d3dFormat);
			viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipLevels = desc.MipLevels;
			viewDesc.Texture2D.MostDetailedMip = 0;

			D3D_CHECK_RESULT(dctx.device->CreateShaderResourceView(m_texture, &viewDesc, &m_view));
			if (hasMips)
				dctx.deviceContext->GenerateMips(m_view);
		}
		else
		{
			if (hasMips)
				Logger::warn("Trying to generate mips but texture is not a shader resource.");
		}
	}
	~D3D11Texture2D()
	{
		if (m_view)
			m_view->Release();
		if (m_texture)
			m_texture->Release();
	}
	void upload(const void* data, uint32_t level) override
	{
		D3D11_BOX box{};
		box.left = 0;
		box.right = m_width;
		box.top = 0;
		box.bottom = m_height;
		box.front = 0;
		box.back = 1;
		dctx.deviceContext->UpdateSubresource(
			m_texture,
			level,
			&box,
			data,
			m_width * m_component,
			0
		);
	}
	void upload(const Rect& rect, const void* data, uint32_t level) override
	{
		D3D11_BOX box{};
		box.left = (UINT)rect.x;
		box.right = (UINT)(rect.x + rect.w);
		box.top = (UINT)rect.y;
		box.bottom = (UINT)(rect.y + rect.h);
		box.front = 0;
		box.back = 1;
		dctx.deviceContext->UpdateSubresource(
			m_texture,
			level,
			&box,
			data,
			m_width * m_component,
			0
		);
	}
	void download(void* data, uint32_t level) override
	{
		/*D3D11_BOX box{};
		box.left = 0;
		box.right = m_width;
		box.top = 0;
		box.bottom = m_height;
		box.front = 0;
		box.back = 1;

		// create staging texture
		if (!m_staging)
		{
			D3D11_TEXTURE2D_DESC desc;
			desc.Width = m_width;
			desc.Height = m_height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = m_d3dFormat;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;

			D3D_CHECK_RESULT(dctx.device->CreateTexture2D(&desc, nullptr, &m_staging));
		}
		dctx.deviceContext->CopySubresourceRegion(
			m_staging, level,
			0, 0, 0,
			m_texture, 0,
			&box
		);
		D3D11_MAPPED_SUBRESOURCE map{};
		D3D_CHECK_RESULT(dctx.deviceContext->Map(m_staging, 0, D3D11_MAP_READ, 0, &map));
		memcpy(data, map.pData, m_width * m_height * 4);
		dctx.deviceContext->Unmap(m_staging, 0);*/
	}
	TextureHandle handle() const override
	{
		return TextureHandle((uintptr_t)m_view);
	}
	void generateMips() override
	{
		if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
			return;
		dctx.deviceContext->GenerateMips(m_view);
		m_flags = m_flags | TextureFlag::GenerateMips;
	}
};

class D3D11Texture2DMultisample : public Texture2DMultisample, public D3D11Texture
{
public:
	D3D11Texture2DMultisample(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		uint8_t samples,
		const void* data
	) :
		Texture2DMultisample(width, height, format, flags),
		D3D11Texture()
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = static_cast<UINT>(samples);
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.BindFlags = 0;

		bool isShaderResource = (TextureFlag::ShaderResource & flags) == TextureFlag::ShaderResource;
		bool isRenderTarget = (TextureFlag::RenderTarget & flags) == TextureFlag::RenderTarget;
		bool hasMips = (TextureFlag::GenerateMips & flags) == TextureFlag::RenderTarget;

		if (hasMips)
		{
			Logger::warn("Cannot generate mips for multisampled texture.");
			hasMips = false;
			flags = flags & ~TextureFlag::GenerateMips;
		}

		if (isShaderResource)
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_FORMAT_SUPPORT_MULTISAMPLE_LOAD;

		m_d3dFormat = d3dformat(format);
		m_component = d3dComponent(format);

		if (isRenderTarget)
		{
			if (isDepth(format))
				desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL | D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET;
			else
				desc.BindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET;
		}
		if (isShaderResource)
			desc.Format = d3dShaderDataFormat(m_d3dFormat);
		else
			desc.Format = m_d3dFormat;

		D3D11_SUBRESOURCE_DATA subResources{};
		subResources.pSysMem = data;
		subResources.SysMemPitch = width * size(format);
		subResources.SysMemSlicePitch = 0;
		D3D11_SUBRESOURCE_DATA* sub = nullptr;
		if (data != nullptr)
			sub = &subResources;
		D3D_CHECK_RESULT(dctx.device->CreateTexture2D(&desc, sub, &m_texture));
		if (isShaderResource)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
			viewDesc.Format = d3dViewFormat(m_d3dFormat);
			viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2DMS;
			viewDesc.Texture2DMS.UnusedField_NothingToDefine;

			D3D_CHECK_RESULT(dctx.device->CreateShaderResourceView(m_texture, &viewDesc, &m_view));
			if (hasMips)
				dctx.deviceContext->GenerateMips(m_view);
		}
		else
		{
			if (hasMips)
				Logger::warn("Trying to generate mips but texture is not a shader resource.");
		}
	}
	~D3D11Texture2DMultisample()
	{
		if (m_view)
			m_view->Release();
		if (m_texture)
			m_texture->Release();
	}
	void upload(const void* data) override
	{
		D3D11_BOX box{};
		box.left = 0;
		box.right = m_width;
		box.top = 0;
		box.bottom = m_height;
		box.front = 0;
		box.back = 1;
		dctx.deviceContext->UpdateSubresource(
			m_texture,
			0,
			&box,
			data,
			m_width * m_component,
			0
		);
	}
	void upload(const Rect& rect, const void* data) override
	{
		D3D11_BOX box{};
		box.left = (UINT)rect.x;
		box.right = (UINT)(rect.x + rect.w);
		box.top = (UINT)rect.y;
		box.bottom = (UINT)(rect.y + rect.h);
		box.front = 0;
		box.back = 1;
		dctx.deviceContext->UpdateSubresource(
			m_texture,
			0,
			&box,
			data,
			m_width * m_component,
			0
		);
	}
	void download(void* data) override
	{
		/*D3D11_BOX box{};
		box.left = 0;
		box.right = m_width;
		box.top = 0;
		box.bottom = m_height;
		box.front = 0;
		box.back = 1;

		// create staging texture
		if (!m_staging)
		{
			D3D11_TEXTURE2D_DESC desc;
			desc.Width = m_width;
			desc.Height = m_height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = m_d3dFormat;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;

			D3D_CHECK_RESULT(dctx.device->CreateTexture2D(&desc, nullptr, &m_staging));
		}
		dctx.deviceContext->CopySubresourceRegion(
			m_staging, 0,
			0, 0, 0,
			m_texture, 0,
			&box
		);
		D3D11_MAPPED_SUBRESOURCE map{};
		D3D_CHECK_RESULT(dctx.deviceContext->Map(m_staging, 0, D3D11_MAP_READ, 0, &map));
		memcpy(data, map.pData, m_width * m_height * 4);
		dctx.deviceContext->Unmap(m_staging, 0);*/
	}
	
	TextureHandle handle() const override
	{
		return TextureHandle((uintptr_t)m_view);
	}
	void generateMips() override
	{
		Logger::error("Cannot generate mips for multisampled texture.");
	}
};


class D3D11TextureCubeMap : public TextureCubeMap, public D3D11Texture
{
public:
	D3D11TextureCubeMap(
		uint32_t width, uint32_t height,
		TextureFormat format, TextureFlag flags,
		const void* px, const void* nx,
		const void* py, const void* ny,
		const void* pz, const void* nz
	) :
		TextureCubeMap(width, height, format, flags),
		D3D11Texture()
	{
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 6;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		desc.BindFlags = 0;

		bool isShaderResource = (TextureFlag::ShaderResource & flags) == TextureFlag::ShaderResource;
		bool isRenderTarget = (TextureFlag::RenderTarget & flags) == TextureFlag::RenderTarget;
		bool hasMips = (TextureFlag::GenerateMips & flags) == TextureFlag::RenderTarget;

		if (hasMips)
		{
			desc.MipLevels = 0;
			desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		if (isShaderResource)
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		m_d3dFormat = d3dformat(format);
		m_component = d3dComponent(format);

		if (isRenderTarget)
		{
			if (isDepth(format))
				desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
			else
				desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}
		if (isShaderResource)
			desc.Format = d3dShaderDataFormat(m_d3dFormat);
		else
			desc.Format = m_d3dFormat;

		D3D11_SUBRESOURCE_DATA data[6];
		const void* datas[6] = { px, nx, py, ny, pz, nz };
		for (size_t i = 0; i < 6; i++)
		{
			data[i].pSysMem = datas[i];
			data[i].SysMemPitch = width * size(format);
			data[i].SysMemSlicePitch = 0;
		}
		D3D11_SUBRESOURCE_DATA* pData = nullptr;
		if (px != nullptr)
			pData = data;
		D3D_CHECK_RESULT(dctx.device->CreateTexture2D(&desc, pData, &m_texture));
		if (isShaderResource)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc{};
			viewDesc.Format = d3dViewFormat(m_d3dFormat);
			viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
			viewDesc.TextureCube.MipLevels = desc.MipLevels;
			viewDesc.TextureCube.MostDetailedMip = 0;

			D3D_CHECK_RESULT(dctx.device->CreateShaderResourceView(m_texture, &viewDesc, &m_view));
			if (hasMips)
				dctx.deviceContext->GenerateMips(m_view);
		}
		else
		{
			if (hasMips)
				Logger::warn("Trying to generate mips but texture is not a shader resource.");
		}
	}
	~D3D11TextureCubeMap()
	{
		if (m_view)
			m_view->Release();
		if (m_texture)
			m_texture->Release();
	}
	void upload(const void* data, uint32_t layer, uint32_t level) override
	{
		D3D11_BOX box{};
		box.left = 0;
		box.right = m_width;
		box.top = 0;
		box.bottom = m_height;
		box.front = 0;
		box.back = 1;
		dctx.deviceContext->UpdateSubresource(
			m_texture,
			D3D11CalcSubresource(level, layer, levels()),
			&box,
			data,
			m_width * m_component,
			0
		);
	}
	void upload(const Rect& rect, const void* data, uint32_t layer, uint32_t level) override
	{
		D3D11_BOX box{};
		box.left = (UINT)rect.x;
		box.right = (UINT)(rect.x + rect.w);
		box.top = (UINT)rect.y;
		box.bottom = (UINT)(rect.y + rect.h);
		box.front = 0;
		box.back = 1;
		dctx.deviceContext->UpdateSubresource(
			m_texture,
			D3D11CalcSubresource(level, layer, levels()),
			&box,
			data,
			m_width * m_component,
			0
		);
	}
	void download(void* data, uint32_t layer, uint32_t level) override
	{
		/*D3D11_BOX box{};
		box.left = 0;
		box.right = m_width;
		box.top = 0;
		box.bottom = m_height;
		box.front = 0;
		box.back = 1;

		// create staging texture
		if (!m_staging)
		{
			D3D11_TEXTURE2D_DESC desc;
			desc.Width = m_width;
			desc.Height = m_height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = m_d3dFormat;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;

			D3D_CHECK_RESULT(dctx.device->CreateTexture2D(&desc, nullptr, &m_staging));
		}
		dctx.deviceContext->CopySubresourceRegion(
			m_staging, 0,
			0, 0, 0,
			m_texture, 0,
			&box
		);
		D3D11_MAPPED_SUBRESOURCE map{};
		D3D_CHECK_RESULT(dctx.deviceContext->Map(m_staging, 0, D3D11_MAP_READ, 0, &map));
		memcpy(data, map.pData, m_width * m_height * 4);
		dctx.deviceContext->Unmap(m_staging, 0);*/
	}
	TextureHandle handle() const override
	{
		return TextureHandle((uintptr_t)m_view);
	}
	void generateMips() override
	{
		if ((TextureFlag::GenerateMips & m_flags) == TextureFlag::GenerateMips)
			return;
		dctx.deviceContext->GenerateMips(m_view);
		m_flags = m_flags | TextureFlag::GenerateMips;
	}
};

D3D11Texture* D3D11Texture::convert(const Texture::Ptr& texture)
{
	switch (texture->type())
	{
	case TextureType::Texture2D:
		return reinterpret_cast<D3D11Texture2D*>(texture.get());
	case TextureType::TextureCubeMap:
		return reinterpret_cast<D3D11TextureCubeMap*>(texture.get());
	case TextureType::Texture2DMultisample:
		return reinterpret_cast<D3D11Texture2DMultisample*>(texture.get());
	default:
		return nullptr;
	}
}

class D3D11Framebuffer : public Framebuffer
{
public:
	D3D11Framebuffer(Attachment* attachments, size_t count) :
		Framebuffer(attachments, count),
		m_colorViews(),
		m_depthStencilView(nullptr)
	{

		for (Attachment& attachment : m_attachments)
		{
			D3D11Texture* d3dTexture = reinterpret_cast<D3D11Texture2D*>(attachment.texture.get());
			if (attachment.type == AttachmentType::Depth || attachment.type == AttachmentType::Stencil || attachment.type == AttachmentType::DepthStencil)
			{
				AKA_ASSERT(m_depthStencilView == nullptr, "Already a depth buffer");
				D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{};
				switch(attachment.texture->type())
				{
				case TextureType::Texture2D:
					viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					viewDesc.Texture2D.MipSlice = 0;
					break;
				case TextureType::Texture2DMultisample:
					viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
					viewDesc.Texture2DMS.UnusedField_NothingToDefine;
					break;
				case TextureType::TextureCubeMap:
					// https://gamedev.net/forums/topic/659535-cubemap-texture-as-depth-buffer-shadowmapping/5171771/
					viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					viewDesc.Texture2DArray.ArraySize = 6;
					viewDesc.Texture2DArray.FirstArraySlice = 0;
					viewDesc.Texture2DArray.MipSlice = 0; // SV_RenderTargetArrayIndex
					break;
				}
				viewDesc.Format = d3dTexture->m_d3dFormat;
				viewDesc.Flags = 0; // D3D11_DSV_READ_ONLY_DEPTH, D3D11_DSV_READ_ONLY_STENCIL
				D3D_CHECK_RESULT(dctx.device->CreateDepthStencilView(d3dTexture->m_texture, &viewDesc, &m_depthStencilView));
			}
			else
			{
				ID3D11RenderTargetView* view = nullptr;
				D3D_CHECK_RESULT(dctx.device->CreateRenderTargetView(d3dTexture->m_texture, nullptr, &view));
				m_colorViews.insert(std::make_pair(attachment.type, view));
			}
		}
	}
	D3D11Framebuffer(const D3D11Framebuffer&) = delete;
	D3D11Framebuffer& operator=(const D3D11Framebuffer&) = delete;
	~D3D11Framebuffer()
	{
		for (auto colorView : m_colorViews)
			colorView.second->Release();

		if (m_depthStencilView)
			m_depthStencilView->Release();
	}
	void clear(const color4f& color, float depth, int stencil, ClearMask mask) override
	{
		if (((int)mask & (int)ClearMask::Color) == (int)ClearMask::Color)
			for (auto view : m_colorViews)
				dctx.deviceContext->ClearRenderTargetView(view.second, color.data);
		UINT flag = 0;
		if (((int)mask & (int)ClearMask::Depth) == (int)ClearMask::Depth)
			flag |= D3D11_CLEAR_DEPTH;
		if (((int)mask & (int)ClearMask::Stencil) == (int)ClearMask::Stencil)
			flag |= D3D11_CLEAR_STENCIL;
		if (m_depthStencilView != nullptr && flag != 0)
			dctx.deviceContext->ClearDepthStencilView(m_depthStencilView, flag, depth, stencil);
	}
	void set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level) override
	{
		// Check attachment
		Attachment newAttachment = Attachment{ type, texture, flag, layer, level };
		if (!valid(newAttachment))
		{
			Logger::error("Incompatible attachment set for framebuffer");
			return;
		}
		Attachment* attachment = getAttachment(type);
		if (attachment == nullptr)
		{
			m_attachments.push_back(newAttachment);
			attachment = &m_attachments.back();
		}
		else
		{
			if (attachment->texture == texture && attachment->flag == flag && attachment->layer == layer && attachment->level == level)
				return; // Everything already set.
			attachment->texture = texture;
			attachment->flag = flag;
			attachment->layer = layer;
			attachment->level = level;
		}

		D3D11Texture* d3dTexture = reinterpret_cast<D3D11Texture2D*>(texture.get());
		if (type == AttachmentType::Depth || type == AttachmentType::Stencil || type == AttachmentType::DepthStencil)
		{
			if (m_depthStencilView != nullptr)
				m_depthStencilView->Release();
			D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{};
			switch (texture->type())
			{
			case TextureType::Texture2D:
				viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				viewDesc.Texture2D.MipSlice = level;
				break;
			case TextureType::Texture2DMultisample:
				viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
				viewDesc.Texture2DMS.UnusedField_NothingToDefine;
				break;
			case TextureType::TextureCubeMap:
				if ((AttachmentFlag::AttachTextureObject & flag) == AttachmentFlag::AttachTextureObject)
				{
					viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					viewDesc.Texture2DArray.ArraySize = 6;
					viewDesc.Texture2DArray.FirstArraySlice = 0;
					viewDesc.Texture2DArray.MipSlice = level;
				}
				else
				{
					viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					viewDesc.Texture2DArray.ArraySize = 1;
					viewDesc.Texture2DArray.FirstArraySlice = layer;
					viewDesc.Texture2DArray.MipSlice = level;
				}
				break;
			default:
				Logger::error("Unsupported texture type");
				break;
			}
			viewDesc.Format = d3dTexture->m_d3dFormat;
			viewDesc.Flags = 0;
			D3D_CHECK_RESULT(dctx.device->CreateDepthStencilView(d3dTexture->m_texture, &viewDesc, &m_depthStencilView));
		}
		else
		{
			D3D11_RENDER_TARGET_VIEW_DESC viewDesc{};
			switch (texture->type())
			{
			case TextureType::Texture2D:
				viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				viewDesc.Texture2D.MipSlice = level;
				break;
			case TextureType::Texture2DMultisample:
				viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
				viewDesc.Texture2DMS.UnusedField_NothingToDefine;
				break;
			case TextureType::TextureCubeMap:
				if ((AttachmentFlag::AttachTextureObject & flag) == AttachmentFlag::AttachTextureObject)
				{
					viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					viewDesc.Texture2DArray.ArraySize = 6;
					viewDesc.Texture2DArray.FirstArraySlice = 0;
					viewDesc.Texture2DArray.MipSlice = level;
				}
				else
				{
					viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					viewDesc.Texture2DArray.ArraySize = 1;
					viewDesc.Texture2DArray.FirstArraySlice = layer;
					viewDesc.Texture2DArray.MipSlice = level;
				}
				break;
			default:
				Logger::error("Unsupported texture type");
				break;
			}
			viewDesc.Format = d3dTexture->m_d3dFormat;
			auto& it = m_colorViews.find(type);
			if (it != m_colorViews.end())
			{
				it->second->Release();
				D3D_CHECK_RESULT(dctx.device->CreateRenderTargetView(d3dTexture->m_texture, &viewDesc, &it->second));
			}
			else
			{
				ID3D11RenderTargetView* view = nullptr;
				D3D_CHECK_RESULT(dctx.device->CreateRenderTargetView(d3dTexture->m_texture, &viewDesc, &view));
				m_colorViews.insert(std::make_pair(type, view));
			}
		}

		// TODO Recompute size
		//computeSize();
	}
	uint32_t getNumberView() const { return static_cast<uint32_t>(m_colorViews.size()); }
	ID3D11RenderTargetView* getRenderTargetView(AttachmentType index) const {
		auto it = m_colorViews.find(index); 
		if (it == m_colorViews.end())
			return nullptr;
		return it->second;
	}
	ID3D11DepthStencilView* getDepthStencilView() const { return m_depthStencilView; }
private:
	std::map<AttachmentType, ID3D11RenderTargetView*> m_colorViews;
	ID3D11DepthStencilView* m_depthStencilView;
};

class D3D11BackBuffer : public Framebuffer, EventListener<BackbufferResizeEvent>
{
public:
	D3D11BackBuffer(uint32_t width, uint32_t height, IDXGISwapChain* sc) :
		Framebuffer(width, height),
		m_swapChain(sc),
		m_renderTargetView(nullptr),
		m_depthStencilView(nullptr)
	{
		ID3D11Texture2D* texture = nullptr;
		D3D_CHECK_RESULT(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture));
		D3D_CHECK_RESULT(dctx.device->CreateRenderTargetView(texture, nullptr, &m_renderTargetView));
		texture->Release();

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
		D3D_CHECK_RESULT(dctx.device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer));

		// Initailze the depth stencil view.
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		// Create the depth stencil view.
		D3D_CHECK_RESULT(dctx.device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));
	}
	D3D11BackBuffer(const D3D11BackBuffer&) = delete;
	D3D11BackBuffer& operator=(const D3D11BackBuffer&) = delete;
	~D3D11BackBuffer()
	{
		if (m_depthStencilView)
			m_depthStencilView->Release();
		if (m_depthStencilBuffer)
			m_depthStencilBuffer->Release();
		if (m_renderTargetView)
			m_renderTargetView->Release();
		// m_swapChain not owned by this class. Do not release.
	}
public:
	void resize(uint32_t width, uint32_t height)
	{
		m_renderTargetView->Release();
		m_depthStencilBuffer->Release();
		m_depthStencilView->Release();

		m_swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

		ID3D11Texture2D* texture = nullptr;
		D3D_CHECK_RESULT(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture));
		D3D_CHECK_RESULT(dctx.device->CreateRenderTargetView(texture, nullptr, &m_renderTargetView));
		texture->Release();
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
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;
		// Create the texture for the depth buffer using the filled out description.
		D3D_CHECK_RESULT(dctx.device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer));

		// Initailze the depth stencil view.
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		// Create the depth stencil view.
		D3D_CHECK_RESULT(dctx.device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));

		m_width = width;
		m_height = height;
	}
	void clear(const color4f& color, float depth, int stencil, ClearMask mask) override
	{
		// Clear the back buffer.
		if (((int)mask & (int)ClearMask::Color) == (int)ClearMask::Color)
			dctx.deviceContext->ClearRenderTargetView(m_renderTargetView, color.data);
		// Clear the depth buffer.
		UINT flag = 0;
		if (((int)mask & (int)ClearMask::Depth) == (int)ClearMask::Depth)
			flag |= D3D11_CLEAR_DEPTH;
		if (((int)mask & (int)ClearMask::Stencil) == (int)ClearMask::Stencil)
			flag |= D3D11_CLEAR_STENCIL;
		if (m_depthStencilView != nullptr && flag != 0)
			dctx.deviceContext->ClearDepthStencilView(m_depthStencilView, flag, depth, stencil);
	}
	void set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level) override
	{
		Logger::error("Cannot set attachment of backbuffer");
		// TODO create Texture as attachment
	}
	void onReceive(const BackbufferResizeEvent& event) override
	{
		if (event.width != 0 && event.height != 0)
			resize(event.width, event.height);
	}
	ID3D11RenderTargetView* getRenderTargetView() const { return m_renderTargetView; }
	ID3D11DepthStencilView* getDepthStencilView() const { return m_depthStencilView; }
	ID3D11Texture2D* getDepthStencilBuffer() const { return m_depthStencilBuffer; } // TODO make it attachment
private:
	IDXGISwapChain* m_swapChain;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11Texture2D* m_depthStencilBuffer;
};

class D3D11Buffer : public Buffer
{
public:
	D3D11Buffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data) :
		Buffer(type, size, usage, access),
		m_buffer(nullptr)
	{
		reallocate(size, data);
	}
	D3D11Buffer(const D3D11Buffer&) = delete;
	D3D11Buffer& operator=(const D3D11Buffer&) = delete;
	~D3D11Buffer()
	{
		if(m_buffer)
			m_buffer->Release();
	}
public:
	void reallocate(size_t size, const void* data = nullptr) override
	{
		if (m_buffer)
			m_buffer->Release();
		D3D11_BUFFER_DESC bufferDesc{};
		switch (m_usage)
		{
		case BufferUsage::Default:
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			break;
		case BufferUsage::Immutable:
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			break;
		case BufferUsage::Dynamic:
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			break;
		case BufferUsage::Staging:
			bufferDesc.Usage = D3D11_USAGE_STAGING;
			break;
		}
		switch (m_type)
		{
		case BufferType::Vertex:
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			break;
		case BufferType::Index:
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			break;
		case BufferType::Uniform:
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			break;
		case BufferType::ShaderStorage:
			bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			break;
		default:
			Logger::warn("Unkwnown buffer type.");
			bufferDesc.BindFlags = 0;
			break;
		}
		if ((m_usage == BufferUsage::Staging) || (m_usage == BufferUsage::Dynamic && (m_access == BufferCPUAccess::Write || m_access == BufferCPUAccess::None)))
		{
			switch (m_access)
			{
			case BufferCPUAccess::Read:
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
				break;
			case BufferCPUAccess::Write:
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				break;
			default:
			case BufferCPUAccess::ReadWrite:
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
				break;
			}
		}
		else
		{
			if (m_access != BufferCPUAccess::None)
				Logger::error("Cannot set given BufferCPUAccess for given BufferUsage.");
			bufferDesc.CPUAccessFlags = 0;
		}
		bufferDesc.ByteWidth = static_cast<UINT>(size);
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA subData{};
		subData.pSysMem = data;
		subData.SysMemPitch = 0;
		subData.SysMemSlicePitch = 0;

		D3D11_SUBRESOURCE_DATA* pSubData = nullptr;
		if (data != nullptr)
			pSubData = &subData;
		D3D_CHECK_RESULT(dctx.device->CreateBuffer(&bufferDesc, pSubData, &m_buffer));
	}

	void upload(const void* data, size_t size, size_t offset = 0) override
	{
		AKA_ASSERT(m_usage != BufferUsage::Dynamic, "Do not work with dynamic buffer. Use map instead.");
		D3D11_BOX box{};
		box.left = (UINT)offset;
		box.right = (UINT)(offset + size);
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;
		dctx.deviceContext->UpdateSubresource(
			m_buffer,
			0,
			&box,
			data,
			0,
			0
		);
	}

	void upload(const void* data) override
	{
		dctx.deviceContext->UpdateSubresource(
			m_buffer,
			0,
			nullptr,
			data,
			0,
			0
		);
	}

	void download(void* data, size_t size, size_t offset = 0) override
	{
		// TODO create staging buffer to download non dynamic buffer
		void* d = map(BufferMap::Read);
		memcpy(data, static_cast<char*>(d) + offset, size);
		unmap();
	}

	void download(void* data) override
	{
		download(data, m_size, 0);
	};

	void* map(BufferMap bufferMap) override
	{
		AKA_ASSERT(m_usage == BufferUsage::Dynamic, "Only works for dynamic buffer. Use upload instead.");
		D3D11_MAP map;
		switch (bufferMap)
		{
		default:
		case BufferMap::Read:
			map = D3D11_MAP_READ;
			break;
		case BufferMap::Write:
			map = D3D11_MAP_WRITE;
			break;
		case BufferMap::ReadWrite:
			map = D3D11_MAP_READ_WRITE;
			break;
		case BufferMap::WriteDiscard:
			map = D3D11_MAP_WRITE_DISCARD;
			break;
		case BufferMap::WriteNoOverwrite:
			map = D3D11_MAP_WRITE_NO_OVERWRITE;
			break;
		}
		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		D3D_CHECK_RESULT(dctx.deviceContext->Map(m_buffer, 0, map, 0, &mappedResource));
		return mappedResource.pData;
	}

	void unmap()  override
	{
		dctx.deviceContext->Unmap(m_buffer, 0);
	}

	BufferHandle handle() const  override
	{
		return BufferHandle((uintptr_t)m_buffer);
	}

	void copy(const Buffer::Ptr& dst, size_t offsetSRC, size_t offsetDST, size_t size) override
	{
		D3D11_BOX box{};
		box.left = (UINT)offsetSRC;
		box.right = (UINT)offsetSRC + (UINT)size;
		box.top = 0;
		box.bottom = 1;
		box.front = 0;
		box.back = 1;

		D3D11_BOX* pBox = nullptr;
		if (offsetSRC != offsetDST)
			pBox = &box;		
		dctx.deviceContext->CopySubresourceRegion(reinterpret_cast<D3D11Buffer*>(dst.get())->m_buffer, 0, (UINT)offsetDST, 0, 0, m_buffer, 0, pBox);
	}
private:
	ID3D11Buffer* m_buffer;
};

class D3D11Mesh : public Mesh
{
public:
	D3D11Mesh() :
		Mesh()
	{

	}
	D3D11Mesh(const D3D11Mesh&) = delete;
	D3D11Mesh& operator=(const D3D11Mesh&) = delete;
	~D3D11Mesh()
	{
	}
public:
	void upload(const VertexAccessor* vertexAccessor, size_t count, const IndexAccessor& indexAccessor) override
	{
		m_vertexAccessors = std::vector<VertexAccessor>(vertexAccessor, vertexAccessor + count);
		m_indexAccessor = indexAccessor;
		m_vertexBuffers.clear();
		m_strides.clear();
		m_offsets.clear();
		for (size_t i = 0; i < count; i++)
		{
			const VertexAccessor& a = vertexAccessor[i];
			m_vertexBuffers.push_back((ID3D11Buffer*)a.bufferView.buffer->handle().value());
			m_strides.push_back(a.bufferView.stride);
			m_offsets.push_back(a.bufferView.offset + a.offset);
		}
		switch (m_indexAccessor.format)
		{
		case IndexFormat::UnsignedByte:
			Logger::error("Unsigned byte format not supported as index buffer.");
			m_indexFormat = DXGI_FORMAT_R8_UINT;
			break;
		case IndexFormat::UnsignedShort:
			m_indexFormat = DXGI_FORMAT_R16_UINT;
			break;
		case IndexFormat::UnsignedInt:
			m_indexFormat = DXGI_FORMAT_R32_UINT;
			break;
		}
	}
	void upload(const VertexAccessor* vertexAccessor, size_t count) override
	{
		m_vertexAccessors = std::vector<VertexAccessor>(vertexAccessor, vertexAccessor + count);
		m_indexAccessor = {};
		m_vertexBuffers.clear();
		m_strides.clear();
		m_offsets.clear();
		for (size_t i = 0; i < count; i++)
		{
			const VertexAccessor& a = vertexAccessor[i];
			m_vertexBuffers.push_back((ID3D11Buffer*)a.bufferView.buffer->handle().value());
			m_strides.push_back(a.bufferView.stride);
			m_offsets.push_back(a.bufferView.offset + a.offset);
		}
	}
	void draw(PrimitiveType type, uint32_t vertexCount, uint32_t vertexOffset) const override
	{
		dctx.deviceContext->IASetVertexBuffers(0, static_cast<UINT>(m_vertexBuffers.size()), m_vertexBuffers.data(), m_strides.data(), m_offsets.data());
		dctx.deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		switch (type)
		{
		default:
		case PrimitiveType::Triangles:
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			break;
		case PrimitiveType::TriangleStrip:
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			break;
		case PrimitiveType::Points:
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			break;
		case PrimitiveType::Lines:
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			break;
		case PrimitiveType::LineStrip:
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
			break;
		case PrimitiveType::LineLoop:
		case PrimitiveType::TriangleFan:
			Logger::error("Primitive type not supported");
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
			break;
		}
		dctx.deviceContext->Draw(vertexCount, vertexOffset);
	}
	void drawIndexed(PrimitiveType type, uint32_t indexCount, uint32_t indexOffset) const override
	{
		dctx.deviceContext->IASetVertexBuffers(0, static_cast<UINT>(m_vertexBuffers.size()), m_vertexBuffers.data(), m_strides.data(), m_offsets.data());
		dctx.deviceContext->IASetIndexBuffer((ID3D11Buffer*)m_indexAccessor.bufferView.buffer->handle().value(), m_indexFormat, m_indexAccessor.bufferView.offset);
		switch (type)
		{
		default:
		case PrimitiveType::Triangles:
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			break;
		case PrimitiveType::TriangleStrip:
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			break;
		case PrimitiveType::Points:
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			break;
		case PrimitiveType::Lines:
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			break;
		case PrimitiveType::LineStrip:
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
			break;
		case PrimitiveType::LineLoop:
		case PrimitiveType::TriangleFan:
			Logger::error("Primitive type not supported");
			dctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
			break;
		}
		dctx.deviceContext->DrawIndexed(indexCount, indexOffset, 0);
	}
private:
	DXGI_FORMAT m_indexFormat;
	std::vector<ID3D11Buffer*> m_vertexBuffers;
	std::vector<UINT> m_strides;
	std::vector<UINT> m_offsets;
};

class D3D11Shader : public Shader
{
public:
	D3D11Shader(ID3D10Blob* shaderBuffer) :
		m_shaderBuffer(shaderBuffer)
	{

	}
	~D3D11Shader()
	{
		if (m_shaderBuffer)
			m_shaderBuffer->Release();
	}
	ID3D10Blob* getShaderBuffer() { return m_shaderBuffer; }
private:
	ID3D10Blob* m_shaderBuffer;
};

class D3D11Program : public Program
{
public:
	D3D11Program(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, Shader::Ptr compute, const VertexAttribute* attributes, size_t count) :
		Program(attributes, count),
		m_layout(nullptr),
		m_vertexShader(nullptr),
		m_pixelShader(nullptr),
		m_geometryShader(nullptr),
		m_computeShader(nullptr)
	{
		auto getShaderBuffer = [](Shader* shader) -> ID3D10Blob* {
			if (shader == nullptr)
				return nullptr;
			return reinterpret_cast<D3D11Shader*>(shader)->getShaderBuffer();
		};
		ID3D10Blob* vertexShaderBuffer = getShaderBuffer(vert.get());
		ID3D10Blob* pixelShaderBuffer = getShaderBuffer(frag.get());
		ID3D10Blob* geometryShaderBuffer = getShaderBuffer(geometry.get());
		ID3D10Blob* computeShaderBuffer = getShaderBuffer(compute.get());
		// --- Create shaders
		if (vert != nullptr)
			D3D_CHECK_RESULT(dctx.device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader));
		if (frag != nullptr)
			D3D_CHECK_RESULT(dctx.device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &m_pixelShader));
		if (compute != nullptr)
			D3D_CHECK_RESULT(dctx.device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), nullptr, &m_computeShader));
		if (geometry != nullptr)
			D3D_CHECK_RESULT(dctx.device->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), nullptr, &m_geometryShader));

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
			D3D_CHECK_RESULT(dctx.device->CreateInputLayout(
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
		std::vector<Uniform> uniformsComp = getUniforms(computeShaderBuffer , ShaderType::Compute);
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
	~D3D11Program()
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

public:
	std::vector<Uniform> getUniforms(ID3D10Blob *shader, ShaderType shaderType)
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
public:
	void use()
	{
		dctx.deviceContext->IASetInputLayout(m_layout);
		dctx.deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
		dctx.deviceContext->GSSetShader(m_geometryShader, nullptr, 0);
		dctx.deviceContext->PSSetShader(m_pixelShader, nullptr, 0);
		dctx.deviceContext->CSSetShader(m_computeShader, nullptr, 0);
	}
private:
	ID3D11InputLayout* m_layout;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11ComputeShader* m_computeShader;
};

class D3D11Material : public Material
{
public:
	D3D11Material(Program::Ptr program) :
		Material(program)
	{
		uint32_t textureCount = 0;
		uint32_t bufferCount = 0;
		for (const Uniform &uniform : *m_program)
		{
			// Create textures & data buffers
			switch (uniform.type)
			{
			case UniformType::Buffer:
				bufferCount = max(bufferCount, uniform.binding + uniform.count);
				break;
			case UniformType::Texture2D:
			case UniformType::TextureCubemap:
			case UniformType::Texture2DMultisample :
				textureCount = max(textureCount, uniform.binding + uniform.count);
				break;
			case UniformType::Int:
			case UniformType::UnsignedInt:
			case UniformType::Float:
			case UniformType::Vec2:
			case UniformType::Vec3:
			case UniformType::Vec4:
			case UniformType::Mat3:
			case UniformType::Mat4:
			default:
				break;
			}
		}
		m_textures.resize(textureCount, nullptr);
		m_samplers.resize(textureCount, TextureSampler::nearest);
		m_buffers.resize(bufferCount, nullptr);
	}
	~D3D11Material()
	{
	}
public:
	void apply()
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
								samplerStates[i].push_back(D3D11Sampler::get(sampler));
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
			dctx.deviceContext->VSSetShaderResources(0, (UINT)shaderResourceViews[0].size(), shaderResourceViews[0].data());
			dctx.deviceContext->VSSetSamplers(0, (UINT)samplerStates[0].size(), samplerStates[0].data());
		}
		if (shaderResourceViews[1].size() > 0)
		{
			AKA_ASSERT(shaderResourceViews[1].size() == samplerStates[1].size(), "Invalid count.");
			dctx.deviceContext->PSSetShaderResources(0, (UINT)shaderResourceViews[1].size(), shaderResourceViews[1].data());
			dctx.deviceContext->PSSetSamplers(0, (UINT)samplerStates[1].size(), samplerStates[1].data());
		}
		if (shaderResourceViews[2].size() > 0)
		{
			AKA_ASSERT(shaderResourceViews[2].size() == samplerStates[2].size(), "Invalid count.");
			dctx.deviceContext->GSSetShaderResources(0, (UINT)shaderResourceViews[2].size(), shaderResourceViews[2].data());
			dctx.deviceContext->GSSetSamplers(0, (UINT)samplerStates[2].size(), samplerStates[2].data());
		}
		if (shaderResourceViews[3].size() > 0)
		{
			AKA_ASSERT(shaderResourceViews[3].size() == samplerStates[3].size(), "Invalid count.");
			dctx.deviceContext->CSSetShaderResources(0, (UINT)shaderResourceViews[3].size(), shaderResourceViews[3].data());
			dctx.deviceContext->CSSetSamplers(0, (UINT)samplerStates[3].size(), samplerStates[3].data());
		}
		// Fill buffer slot from buffers
		if (uniformBuffers[0].size() > 0)
			dctx.deviceContext->VSSetConstantBuffers(0, (UINT)uniformBuffers[0].size(), uniformBuffers[0].data());
		if (uniformBuffers[1].size() > 0)
			dctx.deviceContext->PSSetConstantBuffers(0, (UINT)uniformBuffers[1].size(), uniformBuffers[1].data());
		if (uniformBuffers[2].size() > 0)
			dctx.deviceContext->GSSetConstantBuffers(0, (UINT)uniformBuffers[2].size(), uniformBuffers[2].data());
		if (uniformBuffers[3].size() > 0)
			dctx.deviceContext->GSSetConstantBuffers(0, (UINT)uniformBuffers[3].size(), uniformBuffers[3].data());
	}
};


static std::shared_ptr<D3D11BackBuffer> s_backbuffer = nullptr;

void GraphicBackend::initialize(uint32_t width, uint32_t height)
{
	Device device = Device::getDefault();
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
	if (dctx.swapchain.vsync)
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
	swapChainDesc.OutputWindow = PlatformBackend::getWindowsWindowHandle();
	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	// Set to full screen or windowed mode.
	swapChainDesc.Windowed = !dctx.swapchain.fullscreen;
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
	UINT flags = 0;
#if defined(DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_CHECK_RESULT(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		&featureLevel, 1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&dctx.swapchain.swapChain,
		&dctx.device,
		nullptr,
		&dctx.deviceContext
	));
#if defined(DEBUG)
	D3D_CHECK_RESULT(dctx.device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&dctx.debugInfoQueue));
	D3D_CHECK_RESULT(dctx.debugInfoQueue->PushEmptyStorageFilter());
#endif
	s_backbuffer = std::make_shared<D3D11BackBuffer>(width, height, dctx.swapchain.swapChain);

	// We are using DirectX11 in this backend.
	dctx.features.api = GraphicApi::DirectX11;
	dctx.features.version.major = 11;
	dctx.features.version.minor = 0;
	dctx.features.profile = 50;
	// D3D11_COMMONSHADER_CONSTANT_BUFFER_HW_SLOT_COUNT 
	// D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT 
	dctx.features.maxTextureUnits = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
	dctx.features.maxTextureSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	dctx.features.maxColorAttachments = 0; // ?
	dctx.features.maxElementIndices = 0; // ?
	dctx.features.maxElementVertices =  0; // ?
	dctx.features.coordinates.clipSpacePositive = true; // D3D11 clip space is [0, 1]
	dctx.features.coordinates.originTextureBottomLeft = false; // D3D11 start reading texture at top left.
	dctx.features.coordinates.originUVBottomLeft = false; // D3D11 UV origin is top left
	dctx.features.coordinates.renderAxisYUp = true; // D3D11 render axis y is up
}

void GraphicBackend::destroy()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (dctx.swapchain.swapChain)
	{
		dctx.swapchain.swapChain->SetFullscreenState(false, nullptr);
	}
	if (s_backbuffer)
	{
		s_backbuffer.reset();
	}

	D3D11RasterPass::clear();
	D3D11Sampler::clear();
	D3D11Depth::clear();
	D3D11Blend::clear();

	if (dctx.deviceContext)
	{
		dctx.deviceContext->Release();
		dctx.deviceContext = 0;
	}

	if (dctx.device)
	{
		dctx.device->Release();
		dctx.device = 0;
	}

	if (dctx.swapchain.swapChain)
	{
		dctx.swapchain.swapChain->Release();
		dctx.swapchain.swapChain = 0;
	}
}

GraphicApi GraphicBackend::api()
{
	return dctx.features.api;
}

const GraphicDeviceFeatures& GraphicBackend::features()
{
	return dctx.features;
}

void GraphicBackend::frame()
{
#if defined(DEBUG)
	dctx.log();
#endif
}

void GraphicBackend::present()
{
	// Present the back buffer to the screen since rendering is complete.
	if (dctx.swapchain.vsync)
	{
		// Lock to screen refresh rate.
		dctx.swapchain.swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		dctx.swapchain.swapChain->Present(0, 0);
	}
}

Framebuffer::Ptr GraphicBackend::backbuffer()
{
	return s_backbuffer;
}

void GraphicBackend::render(RenderPass& pass)
{
	{
		// Unbind resources to avoid warning & D3D unbinding texture that is also set as rendertarget
		ID3D11RenderTargetView* nullRenderTarget[5] = { nullptr };
		ID3D11DepthStencilState* nullDepthStencil = nullptr;
		ID3D11ShaderResourceView* const pSRV[10] = { nullptr };
		ID3D11SamplerState* const pSamplers[10] = { nullptr };
		dctx.deviceContext->OMSetRenderTargets(5, nullRenderTarget, nullptr);
		dctx.deviceContext->PSSetShaderResources(0, 10, pSRV);
		dctx.deviceContext->PSSetSamplers(0, 10, pSamplers);
		dctx.deviceContext->VSSetShaderResources(0, 10, pSRV);
		dctx.deviceContext->VSSetSamplers(0, 10, pSamplers);
		dctx.deviceContext->CSSetShaderResources(0, 10, pSRV);
		dctx.deviceContext->CSSetSamplers(0, 10, pSamplers);
		dctx.deviceContext->GSSetShaderResources(0, 10, pSRV);
		dctx.deviceContext->GSSetSamplers(0, 10, pSamplers);
	}
	{
		// Shader
		if (pass.material == nullptr)
		{
			Logger::error("No material set for render pass");
			return;
		}
		else
		{
			D3D11Material* d3dMaterial = (D3D11Material*)pass.material.get();
			d3dMaterial->apply();
		}
	}

	{
		// Rasterizer
		ID3D11RasterizerState* rasterState = D3D11RasterPass::get(pass.cull);
		if (rasterState != nullptr)
			dctx.deviceContext->RSSetState(rasterState);
		else
			dctx.deviceContext->RSSetState(nullptr);
	}

	{
		// Viewport
		if (pass.viewport.w == 0 || pass.viewport.h == 0)
		{
			D3D11_VIEWPORT viewport;
			viewport.Width = static_cast<float>(pass.framebuffer->width());
			viewport.Height = static_cast<float>(pass.framebuffer->height());
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = 0.f;
			viewport.TopLeftY = 0.f;

			// Set the viewport.
			dctx.deviceContext->RSSetViewports(1, &viewport);
		}
		else
		{
			D3D11_VIEWPORT viewport;
			viewport.Width = (float)pass.viewport.w;
			viewport.Height = (float)pass.viewport.h;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = (float)pass.viewport.x;
			viewport.TopLeftY = (float)pass.viewport.y;

			// Set the viewport.
			dctx.deviceContext->RSSetViewports(1, &viewport);
		}
	}

	{
		// Scissor
		if (pass.scissor.w > 0 && pass.scissor.h > 0)
		{
			D3D11_RECT scissor;
			scissor.right = (LONG)pass.scissor.x + pass.scissor.w;
			scissor.bottom = (LONG)pass.scissor.y + pass.scissor.h;
			scissor.left = (LONG)pass.scissor.x;
			scissor.top = (LONG)pass.scissor.y;

			// Set the scissor.
			dctx.deviceContext->RSSetScissorRects(1, &scissor);
		}
	}

	{
		// Set Framebuffer
		if (pass.framebuffer == s_backbuffer)
		{
			D3D11BackBuffer* backbuffer = (D3D11BackBuffer*)s_backbuffer.get();
			ID3D11RenderTargetView* view = backbuffer->getRenderTargetView();
			dctx.deviceContext->OMSetRenderTargets(1, &view, backbuffer->getDepthStencilView());
		}
		else
		{
			D3D11Framebuffer* framebuffer = (D3D11Framebuffer*)pass.framebuffer.get();
			std::vector<ID3D11RenderTargetView*> views;
			for (uint32_t i = 0; i < framebuffer->getNumberView(); i++)
				views.push_back(framebuffer->getRenderTargetView((AttachmentType)((int)AttachmentType::Color0 + i)));
			if (views.size() != 0)
				dctx.deviceContext->OMSetRenderTargets((UINT)views.size(), views.data(), framebuffer->getDepthStencilView());
			else
				dctx.deviceContext->OMSetRenderTargets(0, nullptr, framebuffer->getDepthStencilView());
		}
	}

	{
		// TODO clear
	}

	{
		// Depth
		ID3D11DepthStencilState* depthState = D3D11Depth::get(pass.depth, pass.stencil);
		if (depthState != nullptr)
			dctx.deviceContext->OMSetDepthStencilState(depthState, 1);
		else
			dctx.deviceContext->OMSetDepthStencilState(nullptr, 1);

	}

	{
		// Blend
		ID3D11BlendState *blendState = D3D11Blend::get(pass.blend);
		if (blendState != nullptr)
		{
			float blendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
			dctx.deviceContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);
		}
		else
		{
			dctx.deviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		}
	}

	{
		// Mesh
		if (pass.submesh.mesh == nullptr)
		{
			Logger::error("No mesh set for render pass");
			return;
		}
		else
		{
			if (pass.submesh.mesh->isIndexed())
				pass.submesh.drawIndexed();
			else
				pass.submesh.draw();
		}
	}
}
void GraphicBackend::dispatch(ComputePass& computePass)
{
	throw std::runtime_error("Not implemented");
}
void GraphicBackend::screenshot(const Path& path)
{
	throw std::runtime_error("not implemented");
}

void GraphicBackend::vsync(bool enabled)
{
	dctx.swapchain.vsync = enabled;
}

void GraphicBackend::copy(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST)
{
	AKA_ASSERT(src->format() == dst->format(), "Invalid format");
	AKA_ASSERT(regionSRC.x + regionSRC.width <= src->width() && regionSRC.y + regionSRC.height <= src->height(), "Region not in range.");
	AKA_ASSERT(regionDST.x + regionDST.width <= dst->width() && regionDST.y + regionDST.height <= dst->height(), "Region not in range.");
	AKA_ASSERT(regionSRC.width == regionDST.width && regionSRC.height == regionDST.height, "Region size invalid.");
	
	D3D11_BOX box{};
	box.left = regionSRC.x;
	box.right = regionSRC.x + regionSRC.width;
	box.top = regionSRC.y;
	box.bottom = regionSRC.y + regionSRC.height;
	box.front = 0;
	box.back = 1;

	D3D11_BOX* pBox = nullptr;
	if (regionSRC.width != regionDST.width || regionSRC.height != regionDST.height)
		pBox = &box;

	D3D11Texture* srcTexture = D3D11Texture::convert(src);
	D3D11Texture* dstTexture = D3D11Texture::convert(dst);
	dctx.deviceContext->CopySubresourceRegion(
		dstTexture->m_texture, regionDST.level,
		regionDST.x, regionDST.y, 0,
		srcTexture->m_texture, regionSRC.level,
		pBox
	);
	if ((TextureFlag::GenerateMips & dst->flags()) == TextureFlag::GenerateMips)
		dst->generateMips();
}

void blitColor(const Texture::Ptr& src, const Framebuffer::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter)
{
	const char* vertShader = ""
		"struct vs_out { float4 position : SV_POSITION; float3 texcoord : TEXCOORD; };\n"
		"vs_out main(float2 position : POSITION) : TEXCOORD0\n"
		"{\n"
		"	vs_out output;\n"
		"	output.texcoord = position * 0.5 + 0.5;\n"
		"	output.texcoord.y = 1.f - output.texcoord.y;\n"
		"	output.position = float4(position.x, position.y, 0.0, 1.0);\n"
		"}\n";
	const char* fragShader = ""
		"struct vs_out { float4 position : SV_POSITION; float3 texcoord : TEXCOORD; };\n"
		"Texture2D    u_input : register(t0);\n"
		"SamplerState u_inputSampler : register(s0);\n"
		"float4 main(vs_out input) : SV_Target\n"
		"{\n"
		"	return u_input.Sample(u_inputSampler, input.texcoord);\n"
		"}\n";
	RenderPass pass;
	pass.framebuffer = dst;

	// Setup quad mesh
	float quadVertices[] = {
		-1.f, -1.f, // bottom left corner
		 1.f, -1.f, // bottom right corner
		 1.f,  1.f, // top right corner
		-1.f,  1.f, // top left corner
	};
	uint16_t quadIndices[] = { 0,1,2,0,2,3 };
	VertexAttribute quadAttributes = VertexAttribute{ VertexSemantic::Position, VertexFormat::Float, VertexType::Vec2 };
	pass.submesh.mesh = Mesh::create();
	pass.submesh.mesh->uploadInterleaved(&quadAttributes, 1, quadVertices, 4, IndexFormat::UnsignedShort, quadIndices, 6);
	pass.submesh.type = PrimitiveType::Triangles;
	pass.submesh.offset = 0;
	pass.submesh.count = 6;
	pass.material = Material::create(Program::createVertexProgram(
		Shader::compile(vertShader, ShaderType::Vertex),
		Shader::compile(fragShader, ShaderType::Fragment),
		&quadAttributes, 1
	));
	pass.clear = Clear{ ClearMask::Color | ClearMask::Depth, color4f(0.f), 1.f, 0 };
	pass.blend = Blending::none;
	pass.cull = Culling::none;
	pass.depth = Depth::none;
	pass.stencil = Stencil::none;
	pass.scissor = Rect{ 0 };
	pass.viewport = Rect{ regionDST.x, regionDST.y, regionDST.width, regionDST.height };

	// Setup textures
	TextureSampler sampler;
	sampler.filterMin = filter;
	sampler.filterMag = filter;
	sampler.mipmapMode = TextureMipMapMode::None;
	sampler.wrapU = TextureWrap::ClampToEdge;
	sampler.wrapV = TextureWrap::ClampToEdge;
	sampler.wrapW = TextureWrap::ClampToEdge;
	sampler.anisotropy = 1.f;

	pass.material->set("u_input", sampler);
	pass.material->set("u_input", src);

	pass.execute();
}

void blitDepth(const Texture::Ptr& src, const Framebuffer::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter)
{
	const char* vertShader = ""
		"struct vs_out { float4 position : SV_POSITION; float3 texcoord : TEXCOORD; };\n"
		"vs_out main(float2 position : POSITION) : TEXCOORD0\n"
		"{\n"
		"	vs_out output;\n"
		"	output.texcoord = position * 0.5 + 0.5;\n"
		"	output.texcoord.y = 1.f - output.texcoord.y;\n"
		"	output.position = float4(position.x, position.y, 0.0, 1.0);\n"
		"}\n";
	const char* fragShader = ""
		"struct vs_out { float4 position : SV_POSITION; float3 texcoord : TEXCOORD; };\n"
		"Texture2D    u_input : register(t0);\n"
		"SamplerState u_inputSampler : register(s0);\n"
		"float main(vs_out input) : SV_Depth\n"
		"{\n"
		"	return u_input.Sample(u_inputSampler, input.texcoord).x;\n"
		"}\n";
	RenderPass pass;
	pass.framebuffer = dst;

	// Setup quad mesh
	float quadVertices[] = {
		-1.f, -1.f, // bottom left corner
		 1.f, -1.f, // bottom right corner
		 1.f,  1.f, // top right corner
		-1.f,  1.f, // top left corner
	};
	uint16_t quadIndices[] = { 0,1,2,0,2,3 };
	VertexAttribute quadAttributes = VertexAttribute{ VertexSemantic::Position, VertexFormat::Float, VertexType::Vec2 };
	pass.submesh.mesh = Mesh::create();
	pass.submesh.mesh->uploadInterleaved(&quadAttributes, 1, quadVertices, 4, IndexFormat::UnsignedShort, quadIndices, 6);
	pass.submesh.type = PrimitiveType::Triangles;
	pass.submesh.offset = 0;
	pass.submesh.count = 6;
	pass.material = Material::create(Program::createVertexProgram(
		Shader::compile(vertShader, ShaderType::Vertex),
		Shader::compile(fragShader, ShaderType::Fragment),
		&quadAttributes, 1
	));
	pass.clear = Clear{ ClearMask::Color | ClearMask::Depth, color4f(0.f), 1.f, 0 };
	pass.blend = Blending::none;
	pass.cull = Culling::none;
	pass.depth = Depth::none;
	pass.stencil = Stencil::none;
	pass.scissor = Rect{ 0 };
	pass.viewport = Rect{ regionDST.x, regionDST.y, regionDST.width, regionDST.height };

	// Setup textures
	TextureSampler sampler;
	sampler.filterMin = filter;
	sampler.filterMag = filter;
	sampler.mipmapMode = TextureMipMapMode::None;
	sampler.wrapU = TextureWrap::ClampToEdge;
	sampler.wrapV = TextureWrap::ClampToEdge;
	sampler.wrapW = TextureWrap::ClampToEdge;
	sampler.anisotropy = 1.f;

	pass.material->set("u_input", sampler);
	pass.material->set("u_input", src);

	pass.execute();
}

void blitDepthStencil(const Texture::Ptr& src, const Framebuffer::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter)
{
	// TODO draw on quad to ?
	Texture::Ptr dstTex = dst->get(AttachmentType::DepthStencil);
	AKA_ASSERT(src->format() == dstTex->format(), "Invalid format");
	AKA_ASSERT(regionSRC.x + regionSRC.width <= src->width() && regionSRC.y + regionSRC.height <= src->height(), "Region not in range.");
	AKA_ASSERT(regionDST.x + regionDST.width <= dst->width() && regionDST.y + regionDST.height <= dst->height(), "Region not in range.");
	AKA_ASSERT(regionSRC.width == regionDST.width && regionSRC.height == regionDST.height, "Region size invalid.");

	D3D11_BOX box{};
	box.left = regionSRC.x;
	box.right = regionSRC.x + regionSRC.width;
	box.top = regionSRC.y;
	box.bottom = regionSRC.y + regionSRC.height;
	box.front = 0;
	box.back = 1;

	D3D11_BOX* pBox = nullptr;
	if (regionSRC.width != regionDST.width || regionSRC.height != regionDST.height)
		pBox = &box;
	D3D11Texture* srcTexture = D3D11Texture::convert(src);
	D3D11Texture* dstTexture = D3D11Texture::convert(dstTex);
	dctx.deviceContext->CopySubresourceRegion(
		dstTexture->m_texture, regionDST.level,
		regionDST.x, regionDST.y, 0,
		srcTexture->m_texture, regionSRC.level,
		pBox
	);
}

void GraphicBackend::blit(const Texture::Ptr& src, const Texture::Ptr& dst, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter)
{
	if (src->format() == dst->format() && regionSRC.width == regionDST.width && regionSRC.height == regionDST.height)
	{
		GraphicBackend::copy(src, dst, regionSRC, regionDST);
	}
	else
	{
		if (isDepthStencil(src->format()))
		{
			AKA_ASSERT(isDepthStencil(dst->format()), "");
			Attachment attachment = Attachment{ AttachmentType::DepthStencil, dst, AttachmentFlag::None, regionDST.layer, regionDST.level };
			Framebuffer::Ptr framebuffer = Framebuffer::create(&attachment, 1);
			blitDepthStencil(src, framebuffer, regionSRC, regionDST, filter);
		}
		else if (isDepth(src->format()))
		{
			AKA_ASSERT(isDepth(dst->format()), "");
			Attachment attachment = Attachment{ AttachmentType::Depth, dst, AttachmentFlag::None, regionDST.layer, regionDST.level };
			Framebuffer::Ptr framebuffer = Framebuffer::create(&attachment, 1);
			blitDepth(src, framebuffer, regionSRC, regionDST, filter);
		}
		else
		{
			Attachment attachment = Attachment{ AttachmentType::Color0, dst, AttachmentFlag::None, regionDST.layer, regionDST.level };
			Framebuffer::Ptr framebuffer = Framebuffer::create(&attachment, 1);
			blitColor(src, framebuffer, regionSRC, regionDST, filter);
		}
	}
}

void GraphicBackend::blitToBackbuffer(const Texture::Ptr& src, TextureFilter filter)
{
	TextureRegion regionSRC{ 0, 0, src->width(), src->height(), 0, 0 };
	TextureRegion regionDST{ 0, 0, s_backbuffer->width(), s_backbuffer->height(), 0, 0 };
	blitToBackbuffer(src, regionSRC, regionDST, filter);
}

void GraphicBackend::blitToBackbuffer(const Texture::Ptr& src, const TextureRegion& regionSRC, const TextureRegion& regionDST, TextureFilter filter)
{
	if (isDepthStencil(src->format()))
	{
		AKA_ASSERT(src->format() == TextureFormat::Depth24Stencil8 || src->format() == TextureFormat::DepthStencil, "Invalid format");
		AKA_ASSERT(regionSRC.x + regionSRC.width <= src->width() && regionSRC.y + regionSRC.height <= src->height(), "Region not in range.");
		AKA_ASSERT(regionDST.x + regionDST.width <= s_backbuffer->width() && regionDST.y + regionDST.height <= s_backbuffer->height(), "Region not in range.");
		AKA_ASSERT(regionSRC.width == regionDST.width && regionSRC.height == regionDST.height, "Region size invalid.");

		D3D11_BOX box{};
		box.left = regionSRC.x;
		box.right = regionSRC.x + regionSRC.width;
		box.top = regionSRC.y;
		box.bottom = regionSRC.y + regionSRC.height;
		box.front = 0;
		box.back = 1;

		D3D11_BOX* pBox = nullptr;
		if (regionSRC.width != regionDST.width || regionSRC.height != regionDST.height)
			pBox = &box;

		D3D11Texture* srcTexture = D3D11Texture::convert(src);
		dctx.deviceContext->CopySubresourceRegion(
			s_backbuffer->getDepthStencilBuffer(), regionDST.level,
			regionDST.x, regionDST.y, 0,
			srcTexture->m_texture, regionSRC.level,
			pBox
		);
	}
	else if (isDepth(src->format()))
	{
		blitDepth(src, s_backbuffer, regionSRC, regionDST, filter);
	}
	else
	{
		blitColor(src, s_backbuffer, regionSRC, regionDST, filter);
	}
}

ID3D11Device* GraphicBackend::getD3D11Device()
{
	return dctx.device;
}

ID3D11DeviceContext* GraphicBackend::getD3D11DeviceContext()
{
	return dctx.deviceContext;
}

Device GraphicBackend::getDevice(uint32_t id)
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
	int error = wcstombs_s(&stringLength, device.vendor, 128, adapterDesc.Description, 128);
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

uint32_t GraphicBackend::deviceCount()
{
	return 0;
}

Texture2D::Ptr GraphicBackend::createTexture2D(uint32_t width, uint32_t height, TextureFormat format, TextureFlag flags, const void* data)
{
	// DirectX do not support texture with null size (but opengl does ?).
	if (width == 0 || height == 0)
		return nullptr;
	return std::make_shared<D3D11Texture2D>(width, height, format, flags, data);
}

TextureCubeMap::Ptr GraphicBackend::createTextureCubeMap(
	uint32_t width, uint32_t height, 
	TextureFormat format, TextureFlag flags,
	const void* px, const void* nx,
	const void* py, const void* ny,
	const void* pz, const void* nz
)
{
	// DirectX do not support texture with null size (but opengl does ?).
	if (width == 0 || height == 0)
		return nullptr;
	return std::make_shared<D3D11TextureCubeMap>(width, height, format, flags, px, nx, py, ny, pz, nz);
}

Texture2DMultisample::Ptr GraphicBackend::createTexture2DMultisampled(
	uint32_t width, uint32_t height,
	TextureFormat format, TextureFlag flags,
	uint8_t samples,
	const void* data
)
{
	// DirectX do not support texture with null size (but opengl does ?).
	if (width == 0 || height == 0)
		return nullptr;
	return std::make_shared<D3D11Texture2DMultisample>(width, height, format, flags, samples, data);
}

Framebuffer::Ptr GraphicBackend::createFramebuffer(Attachment* attachments, size_t count)
{
	return std::make_shared<D3D11Framebuffer>(attachments, count);
}

Buffer::Ptr GraphicBackend::createBuffer(BufferType type, size_t size, BufferUsage usage, BufferCPUAccess access, const void* data)
{
	return std::make_shared<D3D11Buffer>(type, size, usage, access, data);
}

Mesh::Ptr GraphicBackend::createMesh()
{
	return std::make_shared<D3D11Mesh>();
}

Shader::Ptr GraphicBackend::compile(const char* content, ShaderType type)
{
	ID3DBlob* shaderBuffer = nullptr;
	ID3DBlob* errorMessage = nullptr;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG)
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

Program::Ptr GraphicBackend::createVertexProgram(Shader::Ptr vert, Shader::Ptr frag, const VertexAttribute* attributes, size_t count)
{
	return std::make_shared<D3D11Program>(vert, frag, nullptr, nullptr, attributes, count);
}

Program::Ptr GraphicBackend::createComputeProgram(Shader::Ptr compute)
{
	return std::make_shared<D3D11Program>(nullptr, nullptr, nullptr, compute, nullptr, 0);
}

Program::Ptr GraphicBackend::createGeometryProgram(Shader::Ptr vert, Shader::Ptr frag, Shader::Ptr geometry, const VertexAttribute* attributes, size_t count)
{
	return std::make_shared<D3D11Program>(vert, frag, geometry, nullptr, attributes, count);
}

Material::Ptr GraphicBackend::createMaterial(Program::Ptr program)
{
	return std::make_shared<D3D11Material>(program);
}

};
#endif