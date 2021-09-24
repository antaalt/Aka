#if defined(AKA_USE_D3D11)
#pragma once

#include "D3D11Context.h"

#include <Aka/Graphic/Framebuffer.h>

namespace aka {

class D3D11Device;

class D3D11Framebuffer : public Framebuffer
{
	friend class D3D11Device;
public:
	D3D11Framebuffer(D3D11Device* device, Attachment* attachments, size_t count);
	~D3D11Framebuffer();
	void clear(const color4f& color, float depth, int stencil, ClearMask mask) override;
	void set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level) override;
	void computeSize();
private:
	D3D11Device* m_device;
	std::map<AttachmentType, ID3D11RenderTargetView*> m_colorViews;
	ID3D11DepthStencilView* m_depthStencilView;
};

};

#endif