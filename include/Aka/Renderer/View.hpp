#pragma once

#include <Aka/Core/Enum.h>
#include <Aka/Core/Geometry.h>
#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

enum class ViewType
{
	Unknown, 

	Color,
	Shadow,

	First = Color,
	Last = Shadow,
};
enum class ViewTypeMask
{
	None	= 0,
	Color	= 1 << 0,
	Shadow	= 1 << 1,

	All		= Color | Shadow
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ViewTypeMask);

enum class ViewHandle : uint32_t { Invalid = (uint32_t)-1 };

// A view has camera data
// Color view + shadow view
// Each instance can be spawn in views.
struct ViewData
{
	mat4f view;
	mat4f projection;
};

struct View
{
	ViewHandle handle;
	ViewType type;
	ViewData data;
	gfx::BufferHandle buffer[gfx::MaxFrameInFlight];
	gfx::DescriptorSetHandle descriptor[gfx::MaxFrameInFlight];
};

};