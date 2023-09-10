#include "VulkanFramebuffer.h"

#include "VulkanTexture.h"
#include "VulkanGraphicDevice.h"
#include "VulkanRenderPass.h"

namespace aka {
namespace gfx {

VulkanFramebuffer::VulkanFramebuffer(const char* name, uint32_t width, uint32_t height, RenderPassHandle renderPass, const Attachment* colors, uint32_t count, const Attachment* depth) :
	Framebuffer(name, width, height, renderPass, colors, count, depth),
	vk_framebuffer(VK_NULL_HANDLE),
	vk_views{}
{
}

void VulkanFramebuffer::create(VulkanGraphicDevice* device)
{
	VulkanRenderPass* vk_renderPass = device->getVk<VulkanRenderPass>(renderPass);
	vk_framebuffer = VulkanFramebuffer::createVkFramebuffer(device, vk_renderPass->vk_renderpass, this, vk_views);
}

void VulkanFramebuffer::destroy(VulkanGraphicDevice* device)
{
	vkDestroyFramebuffer(device->getVkDevice(), vk_framebuffer, nullptr);
	vk_framebuffer = VK_NULL_HANDLE;
	for (VkImageView& view : vk_views)
	{
		vkDestroyImageView(device->getVkDevice(), view, nullptr);
		view = VK_NULL_HANDLE;
	}
	vk_views.clear();
}

VkFramebuffer VulkanFramebuffer::createVkFramebuffer(VulkanGraphicDevice* device, VkRenderPass renderpass, const VulkanFramebuffer* vk_framebuffer, std::vector<VkImageView>& views)
{
	std::vector<VkImageView> vk_attachments(vk_framebuffer->count);
	for (size_t i = 0; i < vk_framebuffer->count; i++)
	{
		const VulkanTexture* vk_texture = device->getVk<VulkanTexture>(vk_framebuffer->colors[i].texture);
		// View dependent on flag.
		if (!vk_texture->hasLayers() || has(vk_framebuffer->colors[i].flag, AttachmentFlag::AttachTextureObject))
		{
			vk_attachments[i] = vk_texture->vk_view;
		}
		else
		{
			// Create new view for framebuffer.
			vk_attachments[i] = VulkanTexture::createVkImageView(
				device->getVkDevice(),
				vk_texture->vk_image,
				VK_IMAGE_VIEW_TYPE_2D_ARRAY,
				VulkanContext::tovk(vk_texture->format),
				VulkanTexture::getAspectFlag(vk_texture->format),
				vk_framebuffer->colors[i].level,
				vk_framebuffer->colors[i].layer
			);
			views.push_back(vk_attachments[i]);
		}
	}
	if (vk_framebuffer->hasDepthStencil())
	{
		const VulkanTexture* vk_texture = device->getVk<VulkanTexture>(vk_framebuffer->depth.texture);
		// View dependent on flag.
		if (!vk_texture->hasLayers() || has(vk_framebuffer->depth.flag, AttachmentFlag::AttachTextureObject))
		{
			vk_attachments.push_back(vk_texture->vk_view);
		}
		else
		{
			// Create new view for framebuffer.
			vk_attachments.push_back(VulkanTexture::createVkImageView(
				device->getVkDevice(),
				vk_texture->vk_image,
				VK_IMAGE_VIEW_TYPE_2D_ARRAY,
				VulkanContext::tovk(vk_texture->format),
				VulkanTexture::getAspectFlag(vk_texture->format),
				1,
				1,
				vk_framebuffer->depth.level,
				vk_framebuffer->depth.layer
			));
			views.push_back(vk_attachments.back());
		}
	}

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderpass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(vk_attachments.size());
	framebufferInfo.pAttachments = vk_attachments.data();
	framebufferInfo.width = vk_framebuffer->width;
	framebufferInfo.height = vk_framebuffer->height;
	framebufferInfo.layers = 1;

	VkFramebuffer vk_framebufferHandle = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateFramebuffer(device->getVkDevice(), &framebufferInfo, nullptr, &vk_framebufferHandle));
	return vk_framebufferHandle;
}

FramebufferHandle VulkanGraphicDevice::createFramebuffer(const char* name, RenderPassHandle handle, const Attachment* attachments, uint32_t count, const Attachment* depth)
{
	AKA_ASSERT(handle != RenderPassHandle::null, "No render pass set.");
	if ((attachments == nullptr && depth == nullptr) || count > FramebufferMaxColorAttachmentCount)
		return FramebufferHandle::null;

	uint32_t width = getWidth(this, attachments, count, depth);
	uint32_t height = getHeight(this, attachments, count, depth);

	VulkanFramebuffer* vk_framebuffer = m_framebufferPool.acquire(name, width, height, handle, attachments, count, depth);

	if (vk_framebuffer == nullptr)
		return FramebufferHandle::null;

	vk_framebuffer->create(this);

	return FramebufferHandle{ vk_framebuffer };
}
void VulkanGraphicDevice::destroy(FramebufferHandle framebuffer)
{
	if (framebuffer == FramebufferHandle::null)
		return;
	VulkanFramebuffer* vk_framebuffer = getVk<VulkanFramebuffer>(framebuffer);

	vk_framebuffer->destroy(this);

	m_framebufferPool.release(vk_framebuffer);
}

void VulkanGraphicDevice::destroy(BackbufferHandle handle)
{
	m_swapchain.destroyBackbuffer(this, handle);
}

BackbufferHandle VulkanGraphicDevice::createBackbuffer(RenderPassHandle handle)
{
	return m_swapchain.createBackbuffer(this, handle);
}

RenderPassHandle VulkanGraphicDevice::createBackbufferRenderPass(AttachmentLoadOp loadOp, AttachmentStoreOp storeOp, ResourceAccessType initialLayout, ResourceAccessType finalLayout)
{
	AKA_ASSERT((loadOp == AttachmentLoadOp::Load) != (initialLayout == ResourceAccessType::Undefined), "Cannot load from undefined layout");
	RenderPassState state{};
	state.addColor(m_swapchain.getColorFormat(), loadOp, storeOp, initialLayout, finalLayout);
	state.setDepth(m_swapchain.getDepthFormat(), loadOp, storeOp, initialLayout, finalLayout);
	return createRenderPass("RenderPassBackbuffer", state);
}

FramebufferHandle VulkanGraphicDevice::get(BackbufferHandle handle, Frame* frame)
{
	VulkanFrame* vk_frame = reinterpret_cast<VulkanFrame*>(frame);
	return m_swapchain.getBackbuffer(this, handle)->handles[vk_frame->m_image.value()];
}

const Framebuffer* VulkanGraphicDevice::get(FramebufferHandle handle)
{
	return static_cast<const Framebuffer*>(handle.__data);
}

void VulkanGraphicDevice::getBackbufferSize(uint32_t& width, uint32_t& height)
{
	// TODO handle when rendering offscreen
	width = m_swapchain.width();
	height = m_swapchain.height();
}

};
};