#include "VulkanFramebuffer.h"

#include "VulkanTexture.h"
#include "VulkanGraphicDevice.h"

namespace aka {
namespace gfx {

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

VkFramebuffer VulkanFramebuffer::createVkFramebuffer(VkDevice device, VkRenderPass renderpass, const Framebuffer* framebuffer, std::vector<VkImageView>& views)
{
	std::vector<VkImageView> vk_attachments(framebuffer->framebuffer.count);
	for (size_t i = 0; i < framebuffer->framebuffer.count; i++)
	{
		const VulkanTexture* vk_texture = reinterpret_cast<const VulkanTexture*>(framebuffer->colors[i].texture.data);
		// View dependent on flag.
		if (!vk_texture->hasLayers() || has(framebuffer->colors[i].flag, AttachmentFlag::AttachTextureObject))
		{
			vk_attachments[i] = vk_texture->vk_view;
		}
		else
		{
			// Create new view for framebuffer.
			vk_attachments[i] = VulkanTexture::createVkImageView(
				device,
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
		const VulkanTexture* vk_texture = reinterpret_cast<const VulkanTexture*>(framebuffer->depth.texture.data);
		// View dependent on flag.
		if (!vk_texture->hasLayers() || has(framebuffer->depth.flag, AttachmentFlag::AttachTextureObject))
		{
			vk_attachments.push_back(vk_texture->vk_view);
		}
		else
		{
			// Create new view for framebuffer.
			vk_attachments.push_back(VulkanTexture::createVkImageView(
				device,
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
	VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &vk_framebuffer));
	return vk_framebuffer;
}

FramebufferHandle VulkanGraphicDevice::createFramebuffer(const Attachment* attachments, uint32_t count, const Attachment* depth)
{
	if ((attachments == nullptr && depth == nullptr) || count > FramebufferState::MaxColorAttachmentCount)
		return FramebufferHandle::null;

	VulkanFramebuffer* framebuffer = m_framebufferPool.acquire();
	if (framebuffer == nullptr)
		return FramebufferHandle::null;
	framebuffer->framebuffer.count = count;
	framebuffer->width = ~0U;
	framebuffer->height = ~0U;
	for (uint32_t i = 0; i < count; i++)
	{
		if (attachments[i].texture.data == nullptr)
			continue;
		framebuffer->width = min(attachments[i].texture.data->width, framebuffer->width);
		framebuffer->height = min(attachments[i].texture.data->height, framebuffer->height);
		framebuffer->colors[i] = attachments[i];
		framebuffer->framebuffer.colors[i].format = attachments[i].texture.data->format;
		framebuffer->framebuffer.colors[i].loadOp = attachments[i].loadOp;
	}
	if (depth == nullptr)
	{
		framebuffer->depth = {};
		framebuffer->framebuffer.depth.format = TextureFormat::Unknown;
	}
	else
	{
		framebuffer->depth = *depth;
		framebuffer->framebuffer.depth.format = depth->texture.data->format;
		framebuffer->framebuffer.depth.loadOp = depth->loadOp;
		framebuffer->width = min(depth->texture.data->width, framebuffer->width);
		framebuffer->height = min(depth->texture.data->height, framebuffer->height);
	}

	framebuffer->isSwapchain = false;
	framebuffer->vk_renderpass = m_context.getRenderPass(framebuffer->framebuffer, VulkanRenderPassLayout::Framebuffer);
	framebuffer->vk_framebuffer = VulkanFramebuffer::createVkFramebuffer(m_context.device, framebuffer->vk_renderpass, framebuffer, framebuffer->vk_views);
	return FramebufferHandle{ framebuffer };
}
void VulkanGraphicDevice::destroy(FramebufferHandle framebuffer)
{
	if (framebuffer.data == nullptr)
		return;
	VulkanFramebuffer* vk_framebuffer = get<VulkanFramebuffer>(framebuffer);
	vkDestroyFramebuffer(m_context.device, vk_framebuffer->vk_framebuffer, nullptr);
	for (VkImageView view : vk_framebuffer->vk_views)
		vkDestroyImageView(m_context.device, view, nullptr);
	vk_framebuffer->vk_renderpass; // Cached. do not destroy here

	m_framebufferPool.release(vk_framebuffer);
}

FramebufferHandle VulkanGraphicDevice::backbuffer(const Frame* frame)
{
	return m_swapchain.backbuffers[frame->image.value];
}

};
};