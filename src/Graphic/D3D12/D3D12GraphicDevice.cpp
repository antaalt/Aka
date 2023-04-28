#include "D3D12GraphicDevice.h"

#if defined(AKA_USE_D3D12)

namespace aka {

D3D12GraphicDevice::D3D12GraphicDevice(PlatformDevice* platform, const GraphicConfig& cfg) :
	m_context(),
	m_swapchain()
{
	m_context.initialize(platform, cfg);
	m_swapchain.initialize(this, platform);
}

D3D12GraphicDevice::~D3D12GraphicDevice()
{
	m_swapchain.shutdown(this);
	m_context.shutdown();
}

GraphicAPI D3D12GraphicDevice::api() const
{
	return GraphicAPI::DirectX12;
}

uint32_t D3D12GraphicDevice::getPhysicalDeviceCount()
{
	return m_context.getPhysicalDeviceCount();
}

PhysicalDevice* D3D12GraphicDevice::getPhysicalDevice(uint32_t index)
{
	return m_context.getPhysicalDevice(index);
}

Frame* D3D12GraphicDevice::frame()
{
	// TODO wait then resize if require resize (shutdown and recreate)
	Frame* frame = m_swapchain.acquireNextImage(this);
	if (frame == nullptr)
	{
		Logger::error("Failed to acquire next swapchain image.");
		return nullptr;
	}
	frame->commandList = acquireCommandList();
	frame->commandList->begin();
	return frame;
}

void D3D12GraphicDevice::present(Frame* frame)
{
	// TODO wait then resize if require resize
	D3D12Frame* vk_frame = reinterpret_cast<D3D12Frame*>(frame);
	frame->commandList->end();
	submit(&vk_frame->commandList, 1);
	m_swapchain.present(this, vk_frame);
	release(vk_frame->commandList);
}

void D3D12GraphicDevice::screenshot(void* data)
{
	// TODO
}

};

#endif