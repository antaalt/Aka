#pragma once

#include <Aka/Graphic/Resource.h>

namespace aka {
namespace gfx {

using FenceValue = uint64_t;

struct Fence;
using FenceHandle = ResourceHandle<Fence>;

struct Fence : Resource
{
	Fence(const char* name);
};

};
};