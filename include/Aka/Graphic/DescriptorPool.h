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
	virtual ~DescriptorPool() {}

	uint32_t size;
	ShaderBindingState bindings;
};

};
};