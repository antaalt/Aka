#pragma once

#include <stdint.h>

namespace aka {
namespace ecs {

// TODO list
// - add system
// - replace lifecycle of components
// - add support for runtime component that does not require serialization.
// - debug draw could be based on components added...
// - Add signals in order to be able to destroy / construct component content.
// - convert renderer to a system... InstanceHandle would then be EntityID (+worldID ?)

constexpr uint32_t MaxEntityCount = 1 << 16; // Store in u16
using EntityType = uint16_t;

enum class ComponentID : uint32_t { Invalid = (EntityType)-1 };
enum class EntityID : EntityType { Invalid = (EntityType)-1 };

};
};