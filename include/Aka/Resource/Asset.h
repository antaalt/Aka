#pragma once

#include <Aka/OS/Path.h>
#include <Aka/Resource/Resource.h>

#include <functional>

namespace aka {

struct Asset
{
	String name;

	Path path; // Path to the asset
	size_t diskSize; // Disk size used by the asset
	ResourceType type; // Type of the asset
	Resource* resource; // Resource if in memory.

	// Check if resource is loaded
	bool isLoaded() const;
	// Load the asset as resource
	bool load(gfx::GraphicDevice* device, bool clearBuildData = false);
	// Destroy the resource
	void unload(gfx::GraphicDevice* device);
	// Save the resource
	bool save();

	// Get the resource of the asset
	template <typename T> T* get();
	template <typename T> const T* get() const;
};

template <typename T> 
T* Asset::get()
{
	if (resource == nullptr)
		return nullptr;
	static_assert(std::is_base_of<Resource, T>::value, "Not a resource.");
	AKA_ASSERT(type == resource->getType(), "Invalid resource type queried.");
	return reinterpret_cast<T*>(resource);
}

template <typename T>
const T* Asset::get() const
{
	if (resource == nullptr)
		return nullptr;
	static_assert(std::is_base_of<Resource, T>::value, "Not a resource.");
	AKA_ASSERT(type == resource->getType(), "Invalid resource type queried.");
	return reinterpret_cast<const T*>(resource);
}

};