#if defined(AKA_USE_VULKAN)

#include <vulkan/vulkan.h>
#include "../VulkanGraphicDevice.h"
#include <Aka/Core/Application.h>

// TODO should store these instead of loading them everytime.
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger
) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator
) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	func(instance, debugMessenger, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkSubmitDebugUtilsMessageEXT(
	VkInstance instance, 
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
	VkDebugUtilsMessageTypeFlagsEXT messageTypes, 
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData
) {
	auto func = (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT");
	func(instance, messageSeverity, messageTypes, pCallbackData);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetDebugUtilsObjectNameEXT(
	VkDevice                                    device,
	const VkDebugUtilsObjectNameInfoEXT* pNameInfo
) {
	auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectNameEXT");
	return func(device, pNameInfo);
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetDebugUtilsObjectTagEXT(
	VkDevice                                    device,
	const VkDebugUtilsObjectTagInfoEXT* pTagInfo
) {
	auto func = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectTagEXT");
	return func(device, pTagInfo);
}

VKAPI_ATTR void VKAPI_CALL vkQueueBeginDebugUtilsLabelEXT(
	VkQueue queue, 
	const VkDebugUtilsLabelEXT* pLabelInfo
) {
	VkDevice device = reinterpret_cast<aka::gfx::VulkanGraphicDevice*>(aka::Application::app()->graphic())->getVkDevice();
	auto func = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkQueueBeginDebugUtilsLabelEXT");
	func(queue, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL vkQueueEndDebugUtilsLabelEXT(
	VkQueue queue
) {
	VkDevice device = reinterpret_cast<aka::gfx::VulkanGraphicDevice*>(aka::Application::app()->graphic())->getVkDevice();
	auto func = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkQueueEndDebugUtilsLabelEXT");
	func(queue);
}

VKAPI_ATTR void VKAPI_CALL vkQueueInsertDebugUtilsLabelEXT(
	VkQueue queue,
	const VkDebugUtilsLabelEXT* pLabelInfo
) {
	VkDevice device = reinterpret_cast<aka::gfx::VulkanGraphicDevice*>(aka::Application::app()->graphic())->getVkDevice();
	auto func = (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkQueueInsertDebugUtilsLabelEXT");
	func(queue, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginDebugUtilsLabelEXT(
	VkCommandBuffer commandBuffer, 
	const VkDebugUtilsLabelEXT* pLabelInfo
) {
	VkDevice device = reinterpret_cast<aka::gfx::VulkanGraphicDevice*>(aka::Application::app()->graphic())->getVkDevice();
	auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT");
	func(commandBuffer, pLabelInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndDebugUtilsLabelEXT(
	VkCommandBuffer commandBuffer
) {
	VkDevice device = reinterpret_cast<aka::gfx::VulkanGraphicDevice*>(aka::Application::app()->graphic())->getVkDevice();
	auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT");
	func(commandBuffer);
}

VKAPI_ATTR void VKAPI_CALL vkCmdInsertDebugUtilsLabelEXT(
	VkCommandBuffer commandBuffer,
	const VkDebugUtilsLabelEXT* pLabelInfo
) {
	VkDevice device = reinterpret_cast<aka::gfx::VulkanGraphicDevice*>(aka::Application::app()->graphic())->getVkDevice();
	auto func = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetDeviceProcAddr(device, "vkCmdInsertDebugUtilsLabelEXT");
	func(commandBuffer, pLabelInfo);
}

#endif