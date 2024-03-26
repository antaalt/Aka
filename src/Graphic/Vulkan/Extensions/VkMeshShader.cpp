#if defined(AKA_USE_VULKAN)

#include <vulkan/vulkan.h>
#include "../VulkanGraphicDevice.h"
#include <Aka/Core/Application.h>

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksEXT(
	VkCommandBuffer                             commandBuffer,
	uint32_t                                    groupCountX,
	uint32_t                                    groupCountY,
	uint32_t                                    groupCountZ)
{
	VkDevice device = reinterpret_cast<aka::gfx::VulkanGraphicDevice*>(aka::Application::app()->graphic())->getVkDevice();
	auto func = (PFN_vkCmdDrawMeshTasksEXT)vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksEXT");
	func(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksIndirectEXT(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	uint32_t                                    drawCount,
	uint32_t                                    stride)
{
	VkDevice device = reinterpret_cast<aka::gfx::VulkanGraphicDevice*>(aka::Application::app()->graphic())->getVkDevice();
	auto func = (PFN_vkCmdDrawMeshTasksIndirectEXT)vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectEXT");
	func(commandBuffer, buffer, offset, drawCount, stride);
}


VKAPI_ATTR void VKAPI_CALL vkCmdDrawMeshTasksIndirectCountEXT(
	VkCommandBuffer                             commandBuffer,
	VkBuffer                                    buffer,
	VkDeviceSize                                offset,
	VkBuffer                                    countBuffer,
	VkDeviceSize                                countBufferOffset,
	uint32_t                                    maxDrawCount,
	uint32_t                                    stride)
{
	VkDevice device = reinterpret_cast<aka::gfx::VulkanGraphicDevice*>(aka::Application::app()->graphic())->getVkDevice();
	auto func = (PFN_vkCmdDrawMeshTasksIndirectCountEXT)vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectCountEXT");
	func(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

#endif