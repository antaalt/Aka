#pragma once

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

enum class ViewID : uint32_t {};

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
	ViewType type;
	ViewData data;
};

};