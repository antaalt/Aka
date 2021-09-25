#if defined(AKA_USE_D3D11)
#pragma once

#include "D3D11Context.h"

#include <Aka/Graphic/Backbuffer.h>
#include <Aka/Core/Event.h>
#include <Aka/Platform/PlatformBackend.h>

namespace aka {

class D3D11Device;

class D3D11Backbuffer : public Backbuffer, public std::enable_shared_from_this<D3D11Backbuffer>
{
	friend class D3D11Device;
public:
	D3D11Backbuffer(D3D11Device* device, IDXGISwapChain* swapchain, uint32_t width, uint32_t height);
	~D3D11Backbuffer();
public:
	// Clear the backbuffer
	void clear(const color4f& color, float depth, int stencil, ClearMask mask) override;

	// Set synchronisation for backbuffer submit
	void set(Synchronisation sync) override;
	// Blit a texture to backbuffer
	void blit(const Texture::Ptr& texture, TextureFilter filter) override;
	// Start a new frame for backbuffer
	void frame() override;
	// Submit current frame for backbuffer
	void submit() override;
	// download current frame content
	void download(void* data) override;
protected: // Can't set backbuffer attachment
	void resize(uint32_t width, uint32_t height) override;
	void set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level) override;
private:
	D3D11Device* m_device;
	IDXGISwapChain* m_swapchain;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11Texture2D* m_depthStencilBuffer;
	bool m_vsync;
};

};

#endif