#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/DescriptorSet.h>

#include "VulkanContext.h"

namespace aka {
namespace gfx {

struct VulkanShader : Shader
{
	VulkanShader(const char* name, ShaderType type);

	VkShaderModule vk_module;
};

struct VulkanProgram : Program
{
	VulkanProgram(const char* name, ShaderHandle vertex, ShaderHandle fragment, const ShaderBindingState* sets, uint32_t bindingCounts);
	VulkanProgram(const char* name, ShaderHandle vertex, ShaderHandle fragment, ShaderHandle geometry, const ShaderBindingState* sets, uint32_t bindingCounts);
	VulkanProgram(const char* name, ShaderHandle compute, const ShaderBindingState* sets, uint32_t bindingCounts);

	VkDescriptorSetLayout vk_descriptorSetLayout[ShaderMaxSetCount];
	VkDescriptorPool vk_descriptorPool[ShaderMaxSetCount];
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