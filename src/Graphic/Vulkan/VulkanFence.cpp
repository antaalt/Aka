#include "VulkanFence.h"

#include "VulkanGraphicDevice.h"

namespace aka {
namespace gfx {

VulkanFence::VulkanFence(const char* name) :
	Fence(name),
	vk_sempahore(VK_NULL_HANDLE)
{
}

void VulkanFence::create(VulkanGraphicDevice* device, FenceValue value)
{
	vk_sempahore = VulkanFence::createVkTimelineSemaphore(device, value);
}

void VulkanFence::destroy(VulkanGraphicDevice* device)
{
	vkDestroySemaphore(device->getVkDevice(), vk_sempahore, getVkAllocator());
	vk_sempahore = VK_NULL_HANDLE;
}

VkSemaphore VulkanFence::createVkTimelineSemaphore(VulkanGraphicDevice* device, FenceValue initialValue)
{
	// https://www.khronos.org/blog/vulkan-timeline-semaphores
	// https://learn.microsoft.com/fr-fr/windows/win32/direct3d12/user-mode-heap-synchronization?redirectedfrom=MSDN
	// https://computergraphics.stackexchange.com/questions/4422/directx-openglvulkan-concepts-mapping-chart

#if !defined(VK_VERSION_1_2)
	#error "Timeline semaphore require Vulkan core 1.2 to work (could use the extension instead)"
#endif

	VkSemaphoreTypeCreateInfo timelineCreateInfo{};
	timelineCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	timelineCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	timelineCreateInfo.initialValue = initialValue;

	VkSemaphoreCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = &timelineCreateInfo;
	createInfo.flags = 0;

	VkSemaphore timelineSemaphore;
	vkCreateSemaphore(device->getVkDevice(), &createInfo, getVkAllocator(), &timelineSemaphore);
	return timelineSemaphore;
}

FenceHandle VulkanGraphicDevice::createFence(const char* name, FenceValue value)
{
	VulkanFence* vk_fence = m_fencePool.acquire(name);
	AKA_ASSERT(vk_fence != nullptr, "Fence failed to be allocated");

	vk_fence->create(this, value);

	return FenceHandle{ vk_fence };
}

void VulkanGraphicDevice::destroy(FenceHandle handle)
{
	if (get(handle) == nullptr)
		return;
	VulkanFence* vk_fence = getVk<VulkanFence>(handle);
	vk_fence->destroy(this);
	m_fencePool.release(vk_fence);
}

const Fence* VulkanGraphicDevice::get(FenceHandle handle)
{
	return static_cast<const Fence*>(handle.__data);
}

void VulkanGraphicDevice::wait(FenceHandle handle, FenceValue waitValue)
{
	VulkanFence* vk_fence = getVk<VulkanFence>(handle);

	VkSemaphoreWaitInfo waitInfo;
	waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	waitInfo.pNext = NULL;
	waitInfo.flags = 0;
	waitInfo.semaphoreCount = 1;
	waitInfo.pSemaphores = &vk_fence->vk_sempahore;
	waitInfo.pValues = &waitValue;

	VK_CHECK_RESULT(vkWaitSemaphores(getVkDevice(), &waitInfo, UINT64_MAX));
}

void VulkanGraphicDevice::signal(FenceHandle handle, FenceValue value)
{
	VulkanFence* vk_fence = getVk<VulkanFence>(handle);

	VkSemaphoreSignalInfo signalInfo;
	signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
	signalInfo.pNext = NULL;
	signalInfo.semaphore = vk_fence->vk_sempahore;
	signalInfo.value = value;

	VK_CHECK_RESULT(vkSignalSemaphore(getVkDevice(), &signalInfo));
}

FenceValue VulkanGraphicDevice::read(FenceHandle handle)
{
	VulkanFence* vk_fence = getVk<VulkanFence>(handle);
	uint64_t value;
	VK_CHECK_RESULT(vkGetSemaphoreCounterValue(getVkDevice(), vk_fence->vk_sempahore, &value));
	return value;
}

};
};
