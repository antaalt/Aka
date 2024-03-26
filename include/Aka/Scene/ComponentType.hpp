#pragma once

#include <Aka/Core/Enum.h>

namespace aka {

// Unique ComponentID for serialization, based on component class name.
// Each component need to implement AKA_DECL_COMPONENT(ComponentName) macro to declare it.
enum class ComponentID : uint32_t { Invalid = (uint32_t)-1 };

enum class ComponentState
{
	PendingActivation,
	Active,
	PendingDestruction,
};

enum class ComponentUpdateFlags : uint32_t
{
	None = 0,

	TransformUpdate = 1 << 0,
	HierarchyUpdate = 1 << 1,
	Update = 1 << 2,
	FixedUpdate = 1 << 3,
	RenderUpdate = 1 << 4,

	All = TransformUpdate | HierarchyUpdate | Update | FixedUpdate | RenderUpdate,
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ComponentUpdateFlags);

using ArchiveComponentVersionType = uint32_t;

};