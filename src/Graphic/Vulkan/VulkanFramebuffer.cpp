#include "VulkanFramebuffer.h"

#include "VulkanTexture.h"
#include "VulkanGraphicDevice.h"

namespace aka {
namespace gfx {

VulkanFramebuffer::VulkanFramebuffer(const char* name, uint32_t width, uint32_t height, const FramebufferState& state, const Attachment* colors, const Attachment* depth) :
	Framebuffer(name, width, height, state, colors, depth),
	vk_renderpass(VK_NULL_HANDLE),
	vk_framebuffer(VK_NULL_HANDLE),
	isSwapchain(false),
	vk_views{}
{
}

void VulkanFramebuffer::create(VulkanGraphicDevice* device)
{
	isSwapchain = false;
	vk_renderpass = device->context().getRenderPass(framebuffer, VulkanRenderPassLayout::Framebuffer);
	vk_framebuffer = VulkanFramebuffer::createVkFramebuffer(device, vk_renderpass, this, vk_views);
}

void VulkanFramebuffer::destroy(VulkanGraphicDevice* device)
{
	vkDestroyFramebuffer(device->context().device, vk_framebuffer, nullptr);
	vk_framebuffer = VK_NULL_HANDLE;
	for (VkImageView& view : vk_views)
	{
		vkDestroyImageView(device->context().device, view, nullptr);
		view = VK_NULL_HANDLE;
	}
	vk_views.clear();
	vk_renderpass = VK_NULL_HANDLE; // Cached. do not destroy here
}

VkRenderPass VulkanFramebuffer::createVkRenderPass(VkDevice device, const FramebufferState& framebufferDesc, VulkanRenderPassLayout layout)
{
	AKA_ASSERT(layout != VulkanRenderPassLayout::Unknown, "");
	bool swapchain = layout == VulkanRenderPassLayout::Backbuffer;
	bool hasDepth = framebufferDesc.depth.format != TextureFormat::Unknown;
	std::vector<VkAttachmentDescription> vk_attachments(framebufferDesc.count, VkAttachmentDescription{});
	std::vector<VkAttachmentReference> vk_colorAttachmentsReferences(framebufferDesc.count, VkAttachmentReference{});
	VkAttachmentReference vk_depthAttachment{};
	for (size_t i = 0; i < framebufferDesc.count; i++)
	{
		VkAttachmentDescription& vk_attachment = vk_attachments[i];
		vk_attachment.format = VulkanContext::tovk(framebufferDesc.colors[i].format);
		vk_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		vk_attachment.loadOp = (framebufferDesc.colors[i].loadOp == AttachmentLoadOp::Load) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
		vk_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		vk_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		vk_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		vk_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		vk_attachment.finalLayout = swapchain ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		vk_colorAttachmentsReferences[i].attachment = static_cast<uint32_t>(i);
		vk_colorAttachmentsReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if (hasDepth)
	{
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = VulkanContext::tovk(framebufferDesc.depth.format);
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = (framebufferDesc.depth.loadOp == AttachmentLoadOp::Load) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;// Should be VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL if rendertarrget & shader resource

		vk_depthAttachment.attachment = static_cast<uint32_t>(vk_attachments.size());
		vk_depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		vk_attachments.push_back(depthAttachment);
	}
	VkSubpassDescription subpassInfo{};
	subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassInfo.colorAttachmentCount = static_cast<uint32_t>(vk_colorAttachmentsReferences.size());
	subpassInfo.pColorAttachments = vk_colorAttachmentsReferences.data();
	if (hasDepth)
		subpassInfo.pDepthStencilAttachment = &vk_depthAttachment;
	subpassInfo.pResolveAttachments = nullptr; // For MSAA

	std::vector<VkSubpassDependency> dependencies;
	//if(framebufferDesc.swapchain)
	{
		dependencies.resize(1);
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = 0;

		if (hasDepth)
		{
			dependencies[0].srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
	}
	/*else
	{
	// https://github.com/SaschaWillems/Vulkan/blob/master/examples/deferred/deferred.cpp
		dependencies.resize(2);
		// Transition in
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = 0;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = 0;
		if (hasDepth)
		{
			dependencies[0].srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		// Transition out
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = 0;
		if (hasDepth)
		{
			dependencies[1].srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[1].dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[1].dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
	}*/

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(vk_attachments.size());
	renderPassInfo.pAttachments = vk_attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassInfo;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	VkRenderPass renderPass = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
	return renderPass;
}

VkFramebuffer VulkanFramebuffer::createVkFramebuffer(VulkanGraphicDevice* device, VkRenderPass renderpass, const Framebuffer* framebuffer, std::vector<VkImageView>& views)
{
	std::vector<VkImageView> vk_attachments(framebuffer->framebuffer.count);
	for (size_t i = 0; i < framebuffer->framebuffer.count; i++)
	{
		const VulkanTexture* vk_texture = device->getVk<VulkanTexture>(framebuffer->colors[i].texture);
		// View dependent on flag.
		if (!vk_texture->hasLayers() || has(framebuffer->colors[i].flag, AttachmentFlag::AttachTextureObject))
		{
			vk_attachments[i] = vk_texture->vk_view;
		}
		else
		{
			// Create new view for framebuffer.
			vk_attachments[i] = VulkanTexture::createVkImageView(
				device->context().device,
				vk_texture->vk_image,
				VK_IMAGE_VIEW_TYPE_2D_ARRAY,
				VulkanContext::tovk(vk_texture->format),
				VulkanTexture::getAspectFlag(vk_texture->format),
				framebuffer->colors[i].level,
				framebuffer->colors[i].layer
			);
			views.push_back(vk_attachments[i]);
		}
	}
	if (framebuffer->hasDepthStencil())
	{
		const VulkanTexture* vk_texture = device->getVk<VulkanTexture>(framebuffer->depth.texture);
		// View dependent on flag.
		if (!vk_texture->hasLayers() || has(framebuffer->depth.flag, AttachmentFlag::AttachTextureObject))
		{
			vk_attachments.push_back(vk_texture->vk_view);
		}
		else
		{
			// Create new view for framebuffer.
			vk_attachments.push_back(VulkanTexture::createVkImageView(
				device->context().device,
				vk_texture->vk_image,
				VK_IMAGE_VIEW_TYPE_2D_ARRAY,
				VulkanContext::tovk(vk_texture->format),
				VulkanTexture::getAspectFlag(vk_texture->format),
				1,
				1,
				framebuffer->depth.level, 
				framebuffer->depth.layer
			));
			views.push_back(vk_attachments.back());
		}
	}

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderpass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(vk_attachments.size());
	framebufferInfo.pAttachments = vk_attachments.data();
	framebufferInfo.width = framebuffer->width;
	framebufferInfo.height = framebuffer->height;
	framebufferInfo.layers = 1;

	VkFramebuffer vk_framebuffer = VK_NULL_HANDLE;
	VK_CHECK_RESULT(vkCreateFramebuffer(device->context().device, &framebufferInfo, nullptr, &vk_framebuffer));
	return vk_framebuffer;
}

FramebufferHandle VulkanGraphicDevice::createFramebuffer(const char* name, const Attachment* attachments, uint32_t count, const Attachment* depth)
{
	if ((attachments == nullptr && depth == nullptr) || count > FramebufferMaxColorAttachmentCount)
		return FramebufferHandle::null;

	FramebufferState state = getState(this, attachments, count, depth);
	uint32_t width = getWidth(this, attachments, count, depth);
	uint32_t height = getHeight(this, attachments, count, depth);

	VulkanFramebuffer* vk_framebuffer = m_framebufferPool.acquire(name, width, height, state, attachments, depth);

	if (vk_framebuffer == nullptr)
		return FramebufferHandle::null;

	vk_framebuffer->create(this);

	return FramebufferHandle{ vk_framebuffer };
}
void VulkanGraphicDevice::destroy(FramebufferHandle framebuffer)
{
	if (framebuffer.__data == nullptr)
		return;
	VulkanFramebuffer* vk_framebuffer = getVk<VulkanFramebuffer>(framebuffer);

	vk_framebuffer->destroy(this);

	m_framebufferPool.release(vk_framebuffer);
}

FramebufferHandle VulkanGraphicDevice::backbuffer(const Frame* frame)
{
	return m_swapchain.backbuffers[frame->image.value];
}

const Framebuffer* VulkanGraphicDevice::get(FramebufferHandle handle)
{
	return handle.__data;
}

};
};