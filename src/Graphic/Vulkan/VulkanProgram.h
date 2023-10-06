#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/DescriptorSet.h>

#include "VulkanContext.h"

namespace aka {
namespace gfx {

class VulkanGraphicDevice;

struct VulkanShader : Shader
{
	VulkanShader(const char* name, ShaderType type);

	VkShaderModule vk_module;
};

struct VulkanProgram : Program
{
	VulkanProgram(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount);
	VulkanProgram(const char* name, ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount);
	VulkanProgram(const char* name, ShaderHandle compute, const ShaderBindingState* sets, uint32_t bindingCounts, const ShaderConstant* constants, uint32_t constantCount);

};


// Could check VK_KHR_push_descriptor, pushing on runtime, need to rework a lot, but could be way easier ? what about D3D12 ?
// also VK_EXT_descriptor_indexing
struct VulkanDescriptorPool : DescriptorPool
{
	VulkanDescriptorPool(const char* name, const ShaderBindingState& bindings, uint32_t size);

	VkDescriptorPool vk_pool;

	void create(VulkanContext& context);
	void destroy(VulkanContext& context);

	static VkDescriptorPool createVkDescriptorPool(VkDevice device, const ShaderBindingState& bindings, uint32_t size);
};

struct VulkanDescriptorSet : DescriptorSet
{
	VulkanDescriptorSet(const char* name, const ShaderBindingState& bindings, DescriptorPoolHandle pool);
	
	VkDescriptorPool vk_pool; // Do not own it.
	VkDescriptorSet vk_descriptorSet;

	void create(VulkanContext& context);
	void destroy(VulkanContext& context);

	static void updateDescriptorSet(VulkanGraphicDevice* device, const DescriptorSet* set, const DescriptorUpdate* updates, size_t size);
	static VkDescriptorSetLayout createVkDescriptorSetLayout(VkDevice device, const ShaderBindingState& bindings);
	static VkDescriptorSet createVkDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout* layouts, uint32_t count);
};


};
};