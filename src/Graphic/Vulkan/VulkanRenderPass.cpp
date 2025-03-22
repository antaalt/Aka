#include "VulkanRenderPass.h"

#include "VulkanGraphicDevice.h"
#include "VulkanTexture.h"

namespace aka {
namespace gfx {

VulkanRenderPass::VulkanRenderPass(const char* name, const RenderPassState& state) :
	RenderPass(name, state),
	vk_renderpass(VK_NULL_HANDLE)
{
}

void VulkanRenderPass::create(VulkanGraphicDevice* device)
{
	vk_renderpass = device->getVkRenderPass(state);
}

void VulkanRenderPass::destroy(VulkanGraphicDevice* device)
{
	vk_renderpass = VK_NULL_HANDLE; // Cached. do not destroy here
}

VkRenderPass VulkanRenderPass::createVkRenderPass(VkDevice device, const RenderPassState& state)
{
	Vector<VkAttachmentDescription> vk_attachments(state.count, VkAttachmentDescription{});
	Vector<VkAttachmentReference> vk_colorAttachmentsReferences(state.count, VkAttachmentReference{});
	VkAttachmentReference vk_depthAttachment{};
	for (size_t i = 0; i < state.count; i++)
	{
		const RenderPassState::Attachment& attachment = state.colors[i];
		VkAttachmentDescription& vk_attachment = vk_attachments[i];
		vk_attachment.format = vk::convert(attachment.format);
		vk_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // TODO multisampling
		vk_attachment.loadOp = vk::convert(attachment.loadOp);
		vk_attachment.storeOp = vk::convert(attachment.storeOp);
		vk_attachment.stencilLoadOp = vk::convert(attachment.loadOp);
		vk_attachment.stencilStoreOp = vk::convert(attachment.storeOp);
		vk_attachment.initialLayout = vk::convert(attachment.initialLayout, attachment.format);
		vk_attachment.finalLayout = vk::convert(attachment.finalLayout, attachment.format);

		vk_colorAttachmentsReferences[i].attachment = static_cast<uint32_t>(i);
		vk_colorAttachmentsReferences[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	if (state.hasDepthStencil())
	{
		const RenderPassState::Attachment& attachment = state.depth;
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = vk::convert(attachment.format);
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = vk::convert(attachment.loadOp);
		depthAttachment.storeOp = vk::convert(attachment.storeOp);
		depthAttachment.stencilLoadOp = vk::convert(attachment.loadOp);
		depthAttachment.stencilStoreOp = vk::convert(attachment.storeOp);
		depthAttachment.initialLayout = vk::convert(attachment.initialLayout, attachment.format);
		depthAttachment.finalLayout = vk::convert(attachment.finalLayout, attachment.format);

		vk_depthAttachment.attachment = static_cast<uint32_t>(vk_attachments.size());
		vk_depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // TODO DEPTH_AND_STENCIL_SEPARATELY

		vk_attachments.append(depthAttachment);
	}
	VkSubpassDescription subpassInfo{};
	subpassInfo.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassInfo.colorAttachmentCount = static_cast<uint32_t>(vk_colorAttachmentsReferences.size());
	subpassInfo.pColorAttachments = vk_colorAttachmentsReferences.data();
	if (state.hasDepthStencil())
		subpassInfo.pDepthStencilAttachment = &vk_depthAttachment;
	subpassInfo.pResolveAttachments = nullptr; // For MSAA

	Vector<VkSubpassDependency> dependencies;
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

		if (state.hasDepthStencil())
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
	VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, getVkAllocator(), &renderPass));
	return renderPass;
}

// ------------------------------------------

RenderPassHandle VulkanGraphicDevice::createRenderPass(const char* name, const RenderPassState& state)
{
	VulkanRenderPass* vk_renderPass = m_renderPassPool.acquire(name, state);
	vk_renderPass->create(this);
	return RenderPassHandle{ vk_renderPass };
}

void VulkanGraphicDevice::destroy(RenderPassHandle handle)
{
	if (handle == RenderPassHandle::null)
		return;
	VulkanRenderPass* vk_renderPass = getVk<VulkanRenderPass>(handle);
	vk_renderPass->destroy(this);

	m_renderPassPool.release(vk_renderPass);
}

const RenderPass* VulkanGraphicDevice::get(RenderPassHandle handle)
{
	return static_cast<const RenderPass*>(handle.__data);
}

};
};