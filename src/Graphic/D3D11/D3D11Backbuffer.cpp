#if defined(AKA_USE_D3D11)
#include "D3D11Backbuffer.h"

#include "D3D11Context.h"
#include "D3D11Device.h"
#include "D3D11Texture.h"
#include "Internal/D3D11Blit.h"

#include <Aka/OS/Logger.h>

namespace aka {

D3D11Backbuffer::D3D11Backbuffer(D3D11Device* device, IDXGISwapChain* swapchain, uint32_t width, uint32_t height) :
	Backbuffer(width, height),
	m_device(device),
	m_swapchain(swapchain),
	m_renderTargetView(nullptr),
	m_depthStencilView(nullptr),
	m_depthStencilBuffer(nullptr),
	m_vsync(true)
{
	ID3D11Texture2D* texture = nullptr;
	D3D_CHECK_RESULT(m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture));
	D3D_CHECK_RESULT(m_device->device()->CreateRenderTargetView(texture, nullptr, &m_renderTargetView));
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
	D3D_CHECK_RESULT(m_device->device()->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer));

	// Initailze the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	// Create the depth stencil view.
	D3D_CHECK_RESULT(m_device->device()->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));
}

D3D11Backbuffer::~D3D11Backbuffer()
{
	if (m_depthStencilBuffer)
		m_depthStencilBuffer->Release();
	if (m_depthStencilView)
		m_depthStencilView->Release();
	if (m_renderTargetView)
		m_renderTargetView->Release();
	if (m_swapchain)
	{
		// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
		m_swapchain->SetFullscreenState(false, nullptr);
		m_swapchain->Release();
	}
}

void D3D11Backbuffer::clear(const color4f& color, float depth, int stencil, ClearMask mask)
{
	// Clear the back buffer.
	if (((int)mask & (int)ClearMask::Color) == (int)ClearMask::Color)
		m_device->context()->ClearRenderTargetView(m_renderTargetView, color.data);
	// Clear the depth buffer.
	UINT flag = 0;
	if (((int)mask & (int)ClearMask::Depth) == (int)ClearMask::Depth)
		flag |= D3D11_CLEAR_DEPTH;
	if (((int)mask & (int)ClearMask::Stencil) == (int)ClearMask::Stencil)
		flag |= D3D11_CLEAR_STENCIL;
	if (m_depthStencilView != nullptr && flag != 0)
		m_device->context()->ClearDepthStencilView(m_depthStencilView, flag, depth, stencil);
}

void D3D11Backbuffer::resize(uint32_t width, uint32_t height)
{
	m_renderTargetView->Release();
	m_depthStencilBuffer->Release();
	m_depthStencilView->Release();

	m_swapchain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	ID3D11Texture2D* texture = nullptr;
	D3D_CHECK_RESULT(m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture));
	D3D_CHECK_RESULT(m_device->device()->CreateRenderTargetView(texture, nullptr, &m_renderTargetView));
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
	D3D_CHECK_RESULT(m_device->device()->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer));

	// Initailze the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	// Create the depth stencil view.
	D3D_CHECK_RESULT(m_device->device()->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));
}

void D3D11Backbuffer::set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level)
{
	Logger::error("Trying to set backbuffer attachement.");
}


void D3D11Backbuffer::set(Synchronisation sync)
{
	m_vsync = sync == Synchronisation::Vertical;
}

void D3D11Backbuffer::blit(const Texture::Ptr& texture, TextureFilter filter)
{
	TextureRegion regionSRC{ 0, 0, texture->width(), texture->height(), 0, 0 };
	TextureRegion regionDST{ 0, 0, m_width, m_height, 0 };
	if (isDepthStencil(texture->format()))
	{
		AKA_ASSERT(texture->format() == TextureFormat::Depth24Stencil8 || texture->format() == TextureFormat::DepthStencil, "Invalid format");
		AKA_ASSERT(regionSRC.x + regionSRC.width <= texture->width() && regionSRC.y + regionSRC.height <= texture->height(), "Region not in range.");
		AKA_ASSERT(regionDST.x + regionDST.width <= m_width && regionDST.y + regionDST.height <= m_height, "Region not in range.");
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

		D3D11Texture* srcTexture = D3D11Texture::convert(texture);
		m_device->context()->CopySubresourceRegion(
			m_depthStencilBuffer, regionDST.level,
			regionDST.x, regionDST.y, 0,
			srcTexture->m_texture, regionSRC.level,
			pBox
		);
	}
	else if (isDepth(texture->format()))
	{
		blitDepth(texture, shared_from_this(), regionSRC, regionDST, filter);
	}
	else
	{
		blitColor(texture, shared_from_this(), regionSRC, regionDST, filter);
	}
}

void D3D11Backbuffer::frame()
{
#if defined(DEBUG)
	m_device->ctx()->log();
#endif
}

void D3D11Backbuffer::submit()
{
	if (m_vsync)
	{
		// Lock to screen refresh rate.
		m_swapchain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		m_swapchain->Present(0, 0);
	}
}

void D3D11Backbuffer::download(void* data)
{
	throw std::runtime_error("not implemented");
}

};

#endif