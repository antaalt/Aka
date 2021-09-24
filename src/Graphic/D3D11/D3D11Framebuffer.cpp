#if defined(AKA_USE_D3D11)
#include "D3D11Framebuffer.h"

#include "D3D11Context.h"
#include "D3D11Texture.h"
#include "D3D11Device.h"

#include <Aka/OS/Logger.h>

namespace aka {

D3D11Framebuffer::D3D11Framebuffer(D3D11Device* device, Attachment* attachments, size_t count) :
	Framebuffer(attachments, count),
	m_device(device),
	m_colorViews(),
	m_depthStencilView(nullptr)
{
	for (Attachment& attachment : m_attachments)
	{
		D3D11Texture* d3dTexture = D3D11Texture::convert(attachment.texture);
		if (attachment.type == AttachmentType::Depth || attachment.type == AttachmentType::Stencil || attachment.type == AttachmentType::DepthStencil)
		{
			AKA_ASSERT(m_depthStencilView == nullptr, "Already a depth buffer");
			D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{};
			switch (attachment.texture->type())
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
			D3D_CHECK_RESULT(m_device->device()->CreateDepthStencilView(d3dTexture->m_texture, &viewDesc, &m_depthStencilView));
		}
		else
		{
			ID3D11RenderTargetView* view = nullptr;
			D3D_CHECK_RESULT(m_device->device()->CreateRenderTargetView(d3dTexture->m_texture, nullptr, &view));
			m_colorViews.insert(std::make_pair(attachment.type, view));
		}
	}
}

D3D11Framebuffer::~D3D11Framebuffer()
{
	for (auto colorView : m_colorViews)
		colorView.second->Release();

	if (m_depthStencilView)
		m_depthStencilView->Release();
}

void D3D11Framebuffer::clear(const color4f& color, float depth, int stencil, ClearMask mask)
{
	if (((int)mask & (int)ClearMask::Color) == (int)ClearMask::Color)
		for (auto view : m_colorViews)
			m_device->context()->ClearRenderTargetView(view.second, color.data);
	UINT flag = 0;
	if (((int)mask & (int)ClearMask::Depth) == (int)ClearMask::Depth)
		flag |= D3D11_CLEAR_DEPTH;
	if (((int)mask & (int)ClearMask::Stencil) == (int)ClearMask::Stencil)
		flag |= D3D11_CLEAR_STENCIL;
	if (m_depthStencilView != nullptr && flag != 0)
		m_device->context()->ClearDepthStencilView(m_depthStencilView, flag, depth, stencil);
}

void D3D11Framebuffer::set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level)
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
		D3D_CHECK_RESULT(m_device->device()->CreateDepthStencilView(d3dTexture->m_texture, &viewDesc, &m_depthStencilView));
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
			D3D_CHECK_RESULT(m_device->device()->CreateRenderTargetView(d3dTexture->m_texture, &viewDesc, &it->second));
		}
		else
		{
			ID3D11RenderTargetView* view = nullptr;
			D3D_CHECK_RESULT(m_device->device()->CreateRenderTargetView(d3dTexture->m_texture, &viewDesc, &view));
			m_colorViews.insert(std::make_pair(type, view));
		}
	}

	// TODO Recompute size
	computeSize();
}

void D3D11Framebuffer::computeSize()
{
	m_width = m_attachments[0].texture->width();
	m_height = m_attachments[0].texture->height();

	for (size_t i = 1; i < m_attachments.size(); ++i)
	{
		if (m_width > m_attachments[i].texture->width())
			m_width = m_attachments[i].texture->width();
		if (m_height > m_attachments[i].texture->height())
			m_height = m_attachments[i].texture->height();
	}
}

};

#endif