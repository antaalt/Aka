#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/DescriptorSet.h>

#include "VulkanContext.h"

namespace aka {
namespace gfx {

struct VulkanShader : Shader
{
	VkShaderModule vk_module;
};

struct VulkanProgram : Program
{
	VkDescriptorSetLayout vk_descriptorSetLayout[ShaderBindingState::MaxSetCount];
	VkDescriptorPool vk_descriptorPool[ShaderBindingState::MaxSetCount];
	//VkDescriptorSet vk_descriptorSet;
	//VkPipelineLayout vk_pipelineLayout; // soft reference

	static void updateDescriptorSet(VkDevice device, const DescriptorSet* set, const DescriptorSetData& data);
	static VkDescriptorSetLayout createVkDescriptorSetLayout(VkDevice device, const ShaderBindingState& bindings, VkDescriptorPool* pool);
	static VkDescriptorSet createVkDescriptorSet(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout* layouts, uint32_t count);
};

struct VulkanDescriptorSet : DescriptorSet
{
	VkDescriptorSet vk_descriptorSet;
};


};
};