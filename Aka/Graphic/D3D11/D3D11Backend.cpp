#if defined(AKA_USE_D3D11)
#include "../GraphicBackend.h"
#include "../../Core/Debug.h"
#include "../../Platform/Platform.h"
#include "../../OS/Logger.h"
#include "../../Platform/PlatformBackend.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <stdexcept>
#include <array>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")


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
       DEBUG_BREAK;\
	}								\
}

namespace aka {

struct D3D11SwapChain {
	IDXGISwapChain* swapChain = nullptr;
	bool vsync = true;
	bool fullscreen = false;
};

struct D3D11Context {
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
};

D3D11Context ctx;
D3D11SwapChain swapChain;

struct D3D11RasterPass {
	CullMode cull;
	ID3D11RasterizerState* rasterState;
	static ID3D11RasterizerState* get(CullMode cull)
	{
		for (D3D11RasterPass& pass : cache)
			if (pass.cull == cull)
				return pass.rasterState;
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		switch (cull)
		{
		case CullMode::None: rasterDesc.CullMode = D3D11_CULL_NONE; break;
		case CullMode::FrontFace: rasterDesc.CullMode = D3D11_CULL_FRONT; break;
		case CullMode::BackFace: rasterDesc.CullMode = D3D11_CULL_BACK; break;
		}
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;

		// Create the rasterizer state from the description we just filled out.
		ID3D11RasterizerState* rasterState;
		HRESULT res = ctx.device->CreateRasterizerState(&rasterDesc, &rasterState);
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
	ID3D11ShaderResourceView* texture;
	ID3D11SamplerState* samplerState;
	static ID3D11SamplerState* get(ID3D11ShaderResourceView* texture, Sampler::Filter filter)
	{
		for (D3D11Sampler& sampler : cache)
			if (sampler.texture == texture)
				return sampler.samplerState;

		D3D11_SAMPLER_DESC desc {};
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

		switch (filter)
		{
		case Sampler::Filter::Nearest: desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
		case Sampler::Filter::Linear: desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
		}

		ID3D11SamplerState* result;
		HRESULT res = ctx.device->CreateSamplerState(&desc, &result);
		if (SUCCEEDED(res))
		{
			D3D11Sampler sampler;
			sampler.texture = texture;
			sampler.samplerState = result;
			cache.push_back(sampler);
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

struct D3D11Depth
{
	DepthCompare compare;
	ID3D11DepthStencilState* depthState;
	static ID3D11DepthStencilState* get(DepthCompare compare)
	{
		for (D3D11Depth& depth : cache)
			if (depth.compare == compare)
				return depth.depthState;

		D3D11_DEPTH_STENCIL_DESC desc {};
		desc.DepthEnable = (compare != DepthCompare::None);
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_NEVER;
		switch (compare)
		{
		case DepthCompare::None: desc.DepthFunc = D3D11_COMPARISON_NEVER; break;
		case DepthCompare::Always: desc.DepthFunc = D3D11_COMPARISON_ALWAYS; break;
		case DepthCompare::Never: desc.DepthFunc = D3D11_COMPARISON_NEVER; break;
		case DepthCompare::Less: desc.DepthFunc = D3D11_COMPARISON_LESS; break;
		case DepthCompare::Equal: desc.DepthFunc = D3D11_COMPARISON_EQUAL; break;
		case DepthCompare::LessOrEqual: desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; break;
		case DepthCompare::Greater: desc.DepthFunc = D3D11_COMPARISON_GREATER; break;
		case DepthCompare::NotEqual: desc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL; break;
		case DepthCompare::GreaterOrEqual: desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL; break;
		}
		ID3D11DepthStencilState* result;
		HRESULT res = ctx.device->CreateDepthStencilState(&desc, &result);
		if (SUCCEEDED(res))
		{
			D3D11Depth depth;
			depth.compare = compare;
			depth.depthState = result;
			cache.push_back(depth);
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
	BlendMode blend;
	ID3D11BlendState* blendState;
	static ID3D11BlendState* get(BlendMode blendMode)
	{
		for (D3D11Blend& blend : cache)
			if (blend.blend == blendMode)
				return blend.blendState;

		D3D11_BLEND_DESC desc{};
		desc.AlphaToCoverageEnable = 0;
		desc.IndependentBlendEnable = 0;

		desc.RenderTarget[0].BlendEnable = blendMode != BlendMode::None;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		if (blendMode != BlendMode::None)
		{
			// TODO upgrade blend to support this
			desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;// blendFactor(blendMode);
			desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;// blendFactor(blendMode);
			desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;// blendFactor(blendMode);
			desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;// blendFactor(blendMode);
		}

		for (uint32_t i = 1; i < 8; i++)
			desc.RenderTarget[i] = desc.RenderTarget[0];

		ID3D11BlendState* result = nullptr;
		HRESULT res = ctx.device->CreateBlendState(&desc, &result);
		if (SUCCEEDED(res))
		{
			D3D11Blend blend;
			blend.blend = blendMode;
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

class D3D11Texture : public Texture
{
public:
	friend class D3D11Framebuffer;
	D3D11Texture(uint32_t width, uint32_t height, Format format, const uint8_t* data, Sampler::Filter filter, bool isFramebuffer) :
		Texture(width, height),
		m_texture(nullptr),
		m_staging(nullptr),
		m_view(nullptr),
		m_component(0),
		m_format(DXGI_FORMAT_UNKNOWN),
		m_isFramebuffer(isFramebuffer)
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

		if (isFramebuffer)
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		else
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		switch (format)
		{
		case Texture::Format::Red:
			m_format = DXGI_FORMAT_R8_UNORM;
			m_component = 1;
			break;
		case Texture::Format::Rgba:
		case Texture::Format::Rgba8:
			m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
			m_component = 4;
			break;
		case Texture::Format::DepthStencil:
			m_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			m_component = 4;
			break;
		default:
			Logger::error("Format not supported");
			break;
		}
		desc.Format = m_format;

		D3D_CHECK_RESULT(ctx.device->CreateTexture2D(&desc, nullptr, &m_texture));
		D3D_CHECK_RESULT(ctx.device->CreateShaderResourceView(m_texture, nullptr, &m_view));
		if (data != nullptr)
			upload(data);
	}
	D3D11Texture(D3D11Texture&) = delete;
	D3D11Texture& operator=(D3D11Texture&) = delete;
	~D3D11Texture()
	{
		if (m_view)
			m_view->Release();
		if (m_texture)
			m_texture->Release();
		if (m_staging)
			m_staging->Release();
	}
	void upload(const uint8_t* data) override
	{
		D3D11_BOX box{};
		box.left = 0;
		box.right = m_width;
		box.top = 0;
		box.bottom = m_height;
		box.front = 0;
		box.back = 1;
		ctx.deviceContext->UpdateSubresource(
			m_texture,
			0,
			&box,
			data,
			m_width * m_component,
			0
		);
	}
	void upload(const Rect& rect, const uint8_t* data) override
	{
		D3D11_BOX box{};
		box.left = (UINT)rect.x;
		box.right = (UINT)rect.w;
		box.top = (UINT)rect.y;
		box.bottom = (UINT)rect.h;
		box.front = 0;
		box.back = 1;
		ctx.deviceContext->UpdateSubresource(
			m_texture,
			0,
			&box,
			data,
			m_width * m_component,
			0
		);
	}
	void download(uint8_t* data) override
	{
		D3D11_BOX box{};
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
			desc.Format = m_format;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;

			D3D_CHECK_RESULT(ctx.device->CreateTexture2D(&desc, nullptr, &m_staging));
		}
		ctx.deviceContext->CopySubresourceRegion(
			m_staging, 0,
			0, 0, 0,
			m_texture, 0,
			&box
		);
		D3D11_MAPPED_SUBRESOURCE map{};
		D3D_CHECK_RESULT(ctx.deviceContext->Map(m_staging, 0, D3D11_MAP_READ, 0, &map));
		memcpy(data, map.pData, m_width * m_height * 4);
		ctx.deviceContext->Unmap(m_staging, 0);
	}
	Handle handle() override
	{
		return Handle((uintptr_t)m_view);
	}
	bool isFramebuffer() override
	{
		return m_isFramebuffer;
	}
	ID3D11Texture2D* getTexture() const { return m_texture; }
	ID3D11ShaderResourceView* getView() const { return m_view; }
private:
	ID3D11Texture2D* m_texture;
	ID3D11Texture2D* m_staging;
	ID3D11ShaderResourceView* m_view;
	DXGI_FORMAT m_format;
	uint32_t m_component;
	bool m_isFramebuffer;
};

class D3D11Framebuffer : public Framebuffer
{
public:
	D3D11Framebuffer(uint32_t width, uint32_t height, AttachmentType* attachments, size_t count, Sampler::Filter filter) :
		Framebuffer(width, height),
		m_filter(filter),
		m_colorViews(),
		m_depthStencilView(nullptr)
	{
		for (size_t i = 0; i < count; i++)
		{
			Texture::Format format;
			switch (attachments[i])
			{
			case AttachmentType::Color0:
			case AttachmentType::Color1:
			case AttachmentType::Color2:
			case AttachmentType::Color3:
				format = Texture::Format::Rgba;
				break;
			case AttachmentType::Depth:
			case AttachmentType::Stencil:
			case AttachmentType::DepthStencil:
				format = Texture::Format::DepthStencil;
				break;
			}
			std::shared_ptr<D3D11Texture> tex = std::make_shared<D3D11Texture>(width, height, format, nullptr, filter, true);
			m_attachments.push_back(Attachment{ attachments[i], tex });
			if (attachments[i] == AttachmentType::Depth || attachments[i] == AttachmentType::Stencil || attachments[i] == AttachmentType::DepthStencil)
			{
				ASSERT(m_depthStencilView == nullptr, "Already a depth buffer");
				D3D_CHECK_RESULT(ctx.device->CreateDepthStencilView(tex->m_texture, nullptr, &m_depthStencilView));
			}
			else
			{
				ID3D11RenderTargetView* view = nullptr;
				D3D_CHECK_RESULT(ctx.device->CreateRenderTargetView(tex->m_texture, nullptr, &view));
				m_colorViews.push_back(view);
			}
		}
	}
	D3D11Framebuffer(const D3D11Framebuffer&) = delete;
	D3D11Framebuffer& operator=(const D3D11Framebuffer&) = delete;
	~D3D11Framebuffer()
	{
		for (ID3D11RenderTargetView* colorView : m_colorViews)
			colorView->Release();

		if (m_depthStencilView)
			m_depthStencilView->Release();
	}
	void resize(uint32_t width, uint32_t height) override
	{
		for (ID3D11RenderTargetView* colorView : m_colorViews)
			colorView->Release();
		m_colorViews.clear();
		if (m_depthStencilView)
			m_depthStencilView->Release();

		for (Attachment& attachment : m_attachments)
		{
			Texture::Format format;
			switch (attachment.type)
			{
			case AttachmentType::Color0:
			case AttachmentType::Color1:
			case AttachmentType::Color2:
			case AttachmentType::Color3:
				format = Texture::Format::Rgba;
				break;
			case AttachmentType::Depth:
			case AttachmentType::Stencil:
			case AttachmentType::DepthStencil:
				format = Texture::Format::DepthStencil;
				break;
			}
			std::shared_ptr<D3D11Texture> tex = std::make_shared<D3D11Texture>(width, height, format, nullptr, m_filter, true);
			attachment.texture = tex;
			if (attachment.type == AttachmentType::Depth || attachment.type == AttachmentType::Stencil || attachment.type == AttachmentType::DepthStencil)
			{
				D3D_CHECK_RESULT(ctx.device->CreateDepthStencilView(tex->m_texture, nullptr, &m_depthStencilView));
			}
			else
			{
				ID3D11RenderTargetView* view = nullptr;
				D3D_CHECK_RESULT(ctx.device->CreateRenderTargetView(tex->m_texture, nullptr, &view));
				m_colorViews.push_back(view);
			}
		}
		m_width = width;
		m_height = height;
	}
	void clear(float r, float g, float b, float a) override
	{
		float color[4];
		color[0] = r;
		color[1] = g;
		color[2] = b;
		color[3] = a;
		for (ID3D11RenderTargetView *view : m_colorViews)
			ctx.deviceContext->ClearRenderTargetView(view, color);
		ctx.deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	Texture::Ptr attachment(AttachmentType type) override
	{
		for (Attachment& attachment : m_attachments)
		{
			if (attachment.type == type)
				return attachment.texture;
		}
		return nullptr;
	}
	uint32_t getNumberView() const { return static_cast<uint32_t>(m_colorViews.size()); }
	ID3D11RenderTargetView* getRenderTargetView(uint32_t index) const { return m_colorViews[index]; }
	ID3D11DepthStencilView* getDepthStencilView() const { return m_depthStencilView; }
private:
	Sampler::Filter m_filter;
	std::vector<Attachment> m_attachments;
	std::vector<ID3D11RenderTargetView*> m_colorViews;
	ID3D11DepthStencilView* m_depthStencilView;
};

class D3D11BackBuffer : public Framebuffer
{
public:
	D3D11BackBuffer(uint32_t width, uint32_t height, IDXGISwapChain* sc) :
		Framebuffer(width, height),
		m_swapChain(sc),
		m_texture(nullptr),
		m_renderTargetView(nullptr),
		m_depthStencilView(nullptr)
	{
		D3D_CHECK_RESULT(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_texture));
		D3D_CHECK_RESULT(ctx.device->CreateRenderTargetView(m_texture, nullptr, &m_renderTargetView));

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
		D3D_CHECK_RESULT(ctx.device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer));

		// Initailze the depth stencil view.
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		// Create the depth stencil view.
		D3D_CHECK_RESULT(ctx.device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));
	}
	D3D11BackBuffer(const D3D11BackBuffer&) = delete;
	D3D11BackBuffer& operator=(const D3D11BackBuffer&) = delete;
	~D3D11BackBuffer()
	{
		if (m_texture)
			m_texture->Release();
		if (m_depthStencilView)
			m_depthStencilView->Release();
		if (m_depthStencilBuffer)
			m_depthStencilBuffer->Release();
		if (m_renderTargetView)
			m_renderTargetView->Release();
		// m_swapChain not owned by this class. Do not release.
	}
	void resize(uint32_t width, uint32_t height) override
	{
		m_texture->Release();
		m_renderTargetView->Release();
		m_depthStencilBuffer->Release();
		m_depthStencilView->Release();
		m_swapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		D3D_CHECK_RESULT(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&m_texture));
		D3D_CHECK_RESULT(ctx.device->CreateRenderTargetView(m_texture, nullptr, &m_renderTargetView));

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
		D3D_CHECK_RESULT(ctx.device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer));

		// Initailze the depth stencil view.
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		// Set up the depth stencil view description.
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		// Create the depth stencil view.
		D3D_CHECK_RESULT(ctx.device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));

		m_width = width;
		m_height = height;
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
	Texture::Ptr attachment(AttachmentType type) override
	{
		// TODO create Texture as attachment
		return nullptr;
	}
	ID3D11RenderTargetView* getRenderTargetView() const { return m_renderTargetView; }
	ID3D11DepthStencilView* getDepthStencilView() const { return m_depthStencilView; }
private:
	IDXGISwapChain* m_swapChain;
	ID3D11Texture2D* m_texture;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11Texture2D* m_depthStencilBuffer;
};

class D3D11Mesh : public Mesh
{
public:
	D3D11Mesh() :
		Mesh(),
		m_format(DXGI_FORMAT_UNKNOWN),
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
		m_vertexStride = vertex.stride();

		if (m_vertexBuffer)
			m_vertexBuffer->Release();

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

		if (m_indexBuffer)
			m_indexBuffer->Release();
		switch (indexFormat)
		{
		case IndexFormat::Uint8:
			// TODO check for support
			m_indexFormat = IndexFormat::Uint8;
			m_indexSize = 1;
			m_format = DXGI_FORMAT_R8_UINT;
			break;
		case IndexFormat::Uint16:
			m_indexFormat = IndexFormat::Uint16;
			m_format = DXGI_FORMAT_R16_UINT;
			m_indexSize = 2;
			break;
		case IndexFormat::Uint32:
			m_indexFormat = IndexFormat::Uint32;
			m_format = DXGI_FORMAT_R32_UINT;
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
		ctx.deviceContext->IASetIndexBuffer(m_indexBuffer, m_format, 0);
		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		ctx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// Draw indexed primitives
		ctx.deviceContext->DrawIndexed(indexCount, indexOffset, 0);
	}
private:
	DXGI_FORMAT m_format;
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
		m_vertexUniformValues.resize(m_vertexUniformBuffers.size());
		m_fragmentUniformValues.resize(m_fragmentUniformBuffers.size());
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
		for (ID3D11Buffer* buffer : m_vertexUniformBuffers)
			buffer->Release();
		for (ID3D11Buffer* buffer : m_fragmentUniformBuffers)
			buffer->Release();
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
			for (uint32_t j = 0; j < desc.Variables; j++)
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
		ctx.deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
		if (m_vertexUniformBuffers.size() > 0)
		{
			for (uint32_t iBuffer = 0; iBuffer < m_vertexUniformBuffers.size(); iBuffer++)
			{
				ASSERT(m_vertexUniformValues.size() > 0, "No data for uniform buffer");
				D3D11_MAPPED_SUBRESOURCE mappedResource{};
				D3D_CHECK_RESULT(ctx.deviceContext->Map(m_vertexUniformBuffers[iBuffer], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
				memcpy(mappedResource.pData, m_vertexUniformValues[iBuffer].data(), sizeof(float) * m_vertexUniformValues[iBuffer].size());
				ctx.deviceContext->Unmap(m_vertexUniformBuffers[iBuffer], 0);
			}
			ctx.deviceContext->VSSetConstantBuffers(0, (UINT)m_vertexUniformBuffers.size(), m_vertexUniformBuffers.data());
		}
		// Pixel shader
		ctx.deviceContext->PSSetShader(m_pixelShader, nullptr, 0);
		if (m_fragmentUniformBuffers.size() > 0)
		{
			for (uint32_t iBuffer = 0; iBuffer < m_fragmentUniformBuffers.size(); iBuffer++)
			{
				ASSERT(m_fragmentUniformValues.size() > 0, "No data for uniform buffer");
				D3D11_MAPPED_SUBRESOURCE mappedResource{};
				D3D_CHECK_RESULT(ctx.deviceContext->Map(m_fragmentUniformBuffers[iBuffer], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
				memcpy(mappedResource.pData, m_fragmentUniformValues[iBuffer].data(), sizeof(float) * m_fragmentUniformValues[iBuffer].size());
				ctx.deviceContext->Unmap(m_fragmentUniformBuffers[iBuffer], 0);
			}
			ctx.deviceContext->PSSetConstantBuffers(0, (UINT)m_fragmentUniformBuffers.size(), m_fragmentUniformBuffers.data());
		}
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
			(UINT)polygonLayout.size(),
			m_vertexShaderBuffer->GetBufferPointer(),
			m_vertexShaderBuffer->GetBufferSize(),
			&m_layout
		));
		if (m_vertexShaderBuffer)
		{
			m_vertexShaderBuffer->Release();
			m_vertexShaderBuffer = nullptr;
		}
		if (m_pixelShaderBuffer)
		{
			m_pixelShaderBuffer->Release();
			m_pixelShaderBuffer = nullptr;
		}
		if (m_computeShaderBuffer)
		{
			m_computeShaderBuffer->Release();
			m_computeShaderBuffer = nullptr;
		}
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
	std::vector<std::vector<float>> m_fragmentUniformValues;
};

static std::shared_ptr<D3D11BackBuffer> s_backbuffer = nullptr;

void GraphicBackend::initialize(uint32_t width, uint32_t height)
{
	Device device = getDevice(0);
	Logger::info("Device : ", device.vendor, " - ", device.memory);

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
	swapChainDesc.OutputWindow = PlatformBackend::getD3DHandle();
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
	s_backbuffer = std::make_shared<D3D11BackBuffer>(width, height, swapChain.swapChain);
}

void GraphicBackend::destroy()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (swapChain.swapChain)
	{
		swapChain.swapChain->SetFullscreenState(false, nullptr);
	}

	D3D11RasterPass::clear();
	D3D11Sampler::clear();
	D3D11Depth::clear();
	D3D11Blend::clear();

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

GraphicApi GraphicBackend::api()
{
	return GraphicApi::DirectX11;
}

void GraphicBackend::resize(uint32_t width, uint32_t height)
{
	s_backbuffer->resize(width, height);
}

void GraphicBackend::frame()
{
}

void GraphicBackend::present()
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

void GraphicBackend::viewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
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

Framebuffer::Ptr GraphicBackend::backbuffer()
{
	return s_backbuffer;
}

void GraphicBackend::render(RenderPass& pass)
{
	{
		// Set Framebuffer
		if (pass.framebuffer == s_backbuffer)
		{
			D3D11BackBuffer* backbuffer = (D3D11BackBuffer*)s_backbuffer.get();
			ID3D11RenderTargetView* view = backbuffer->getRenderTargetView();
			ctx.deviceContext->OMSetRenderTargets(1, &view, backbuffer->getDepthStencilView());
		}
		else
		{
			D3D11Framebuffer* framebuffer = (D3D11Framebuffer*)pass.framebuffer.get();
			ID3D11RenderTargetView* view = framebuffer->getRenderTargetView(0);
			ctx.deviceContext->OMSetRenderTargets((UINT)framebuffer->getNumberView(), &view, framebuffer->getDepthStencilView());
		}
	}
	{
		// Rasterizer
		ID3D11RasterizerState* rasterState = D3D11RasterPass::get(pass.cull);
		if (rasterState != nullptr)
			ctx.deviceContext->RSSetState(rasterState);
	}

	{
		// Depth
		ID3D11DepthStencilState* depthState = D3D11Depth::get(pass.depth);
		if (depthState != nullptr)
			ctx.deviceContext->OMSetDepthStencilState(depthState, 1);
	}

	{
		// Blend
		ID3D11BlendState *blendState = D3D11Blend::get(pass.blend);
		if (blendState != nullptr)
		{
			float blendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
			ctx.deviceContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);
		}
		else
		{
			ctx.deviceContext->OMSetBlendState(nullptr, nullptr, 0);
		}
	}

	{
		// Viewport
		D3D11_VIEWPORT viewport;
		viewport.Width = (float)pass.viewport.w;
		viewport.Height = (float)pass.viewport.h;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = pass.viewport.x;
		viewport.TopLeftY = pass.viewport.y;

		// Create the viewport.
		ctx.deviceContext->RSSetViewports(1, &viewport);
	}

	{
		// Shader
		if (pass.shader == nullptr)
		{
			Logger::error("No shader set for render pass");
			return;
		}
		else
		{
			((D3D11Shader*)pass.shader.get())->setLayout(pass.mesh->getVertexData());
			pass.shader->use();
			// Textures
			//for (uint32_t iTex = 0; iTex < textures.size(); iTex++)
			{
				if (pass.texture != nullptr)
				{
					// Assign the Texture
					ID3D11ShaderResourceView* view = ((D3D11Texture*)pass.texture.get())->getView();
					ctx.deviceContext->PSSetShaderResources(0, 1, &view);
				}
			}
			// Fragment Shader Samplers
			//for (int i = 0; i < samplers.size(); i++)
			{
				ID3D11ShaderResourceView* view = ((D3D11Texture*)pass.texture.get())->getView();
				ID3D11SamplerState* sampler = D3D11Sampler::get(view, Sampler::Filter::Nearest);
				if (sampler != nullptr)
					ctx.deviceContext->PSSetSamplers(0, 1, &sampler);
			}
		}
	}
	{
		// Mesh
		if (pass.mesh == nullptr)
		{
			Logger::error("No mesh set for render pass");
			return;
		}
		else
		{
			pass.mesh->draw(pass.indexCount, pass.indexOffset);
		}
	}
}
void GraphicBackend::screenshot(const Path& path)
{
	throw std::runtime_error("not implemented");
}

void GraphicBackend::vsync(bool enabled)
{
	swapChain.vsync = enabled;
}

ID3D11Device* GraphicBackend::getD3D11Device()
{
	return ctx.device;
}

ID3D11DeviceContext* GraphicBackend::getD3D11DeviceContext()
{
	return ctx.deviceContext;
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

Texture::Ptr GraphicBackend::createTexture(uint32_t width, uint32_t height, Texture::Format format, const uint8_t* data, Sampler::Filter filter)
{
	// DirectX do not support texture with null size (but opengl does ?).
	if (width == 0 || height == 0)
		return nullptr;
	return std::make_shared<D3D11Texture>(width, height, format, data, filter, false);
}

Framebuffer::Ptr GraphicBackend::createFramebuffer(uint32_t width, uint32_t height, Framebuffer::AttachmentType* attachment, size_t count, Sampler::Filter filter)
{
	return std::make_shared<D3D11Framebuffer>(width, height, attachment, count, filter);
}

Mesh::Ptr GraphicBackend::createMesh()
{
	return std::make_shared<D3D11Mesh>();
}

ShaderID GraphicBackend::compile(const char* content, ShaderType type)
{
	ID3DBlob* shaderBuffer = nullptr;
	ID3DBlob* errorMessage = nullptr;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
	std::string entryPoint;
	std::string version;
	switch (type)
	{
	case ShaderType::Vertex:
		entryPoint = "vs_main";
		version = "vs_5_0";
		break;
	case ShaderType::Fragment:
		entryPoint = "ps_main";
		version = "ps_5_0";
		break;
	case ShaderType::Compute:
		entryPoint = "";
		version = "";
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

Shader::Ptr GraphicBackend::createShader(ShaderID vert, ShaderID frag, ShaderID compute, const std::vector<Attributes>& attributes)
{
	return std::make_shared<D3D11Shader>(vert, frag, compute, attributes);
}


};
#endif