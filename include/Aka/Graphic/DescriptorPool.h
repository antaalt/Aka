#pragma once

#include <Aka/Graphic/Program.h>
#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

struct DescriptorPool;
using DescriptorPoolHandle = ResourceHandle<DescriptorPool>;

struct DescriptorPool : Resource
{
	DescriptorPool(const char* name, const ShaderBindingState& bindings, uint32_t size);

	uint32_t size;
	ShaderBindingState bindings;

	static DescriptorPoolHandle create(const char* name, const ShaderBindingState& state, uint32_t size);
	static void destroy(DescriptorPoolHandle set);
};

};
};