#pragma once

#include <stdint.h>
#include <type_traits>

#include <Aka/Core/Enum.h>

namespace aka {
namespace gfx {

enum class ResourceType : uint8_t
{
	Unknown,

	Buffer,
	Texture,
	Framebuffer,
	Pipeline,
	DescriptorSet,
	DescriptorPool,
	Sampler,
	Program,
	Shader,
	RenderPass,
	Fence,
};

enum class ResourceAccessType
{
	Undefined,			// No access
	Resource,			// Access as SRV
	Attachment,			// Access as attachment
	Storage,			// Access as UAV
	CopySRC,			// Access as copy
	CopyDST,			// Access as copy
	Present,			// Access as swapchain
};

using ResourceNativeHandle = uint64_t;
constexpr ResourceNativeHandle ResourceNativeHandleInvalid = ~0ULL;

struct Resource
{
	Resource(const char* name, ResourceType type);

	char name[32]; // Debug name of the resource.
	ResourceNativeHandle native; // Native handle of the resource for external API
	ResourceType type; // Type of the resource
};

template <typename T>
struct ResourceHandle
{
	static_assert(std::is_base_of<Resource, T>::value, "Not a resource");
	union {
		const void* __data; // TODO should be obfuscated & renamed desc.
		const T* __typedData; // For debug
	};
	static const ResourceHandle<T> null;
};

template <typename T> 
bool operator==(const ResourceHandle<T>& lhs, const ResourceHandle<T>& rhs) { return lhs.__data == rhs.__data; }
template <typename T>
bool operator!=(const ResourceHandle<T>& lhs, const ResourceHandle<T>& rhs) { return lhs.__data != rhs.__data; }

template <typename T>
const ResourceHandle<T> ResourceHandle<T>::null = { nullptr };

};
};



template <typename T>
struct std::hash<aka::gfx::ResourceHandle<T>>
{
	size_t operator()(const aka::gfx::ResourceHandle<T>& data) const
	{
		size_t hash = 0;
		aka::hashCombine(hash, data.__data);
		return hash;
	}
};
