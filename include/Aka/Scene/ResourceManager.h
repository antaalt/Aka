#pragma once

#include <Aka/Scene/ResourceAllocator.h>

namespace aka {

struct ResourceManager
{
	// Get the allocator of the given resource
	template <typename T> static ResourceAllocator<T>& allocator();
	// Get the raw resource
	template <typename T> static std::shared_ptr<T> get(const String& name);
	// Get the resource 
	template <typename T> static Resource<T> getResource(const String& name);
	// Load a resource from file
	template <typename T> static Resource<T> load(const String& name, const Path& path);
	// Load a resource internally
	template <typename T> static Resource<T> load(const String& name, T&& data);
	// Unload a resource
	template <typename T> static void unload(const String& name);
	// Get the number of resources
	template <typename T> static size_t count();
	// Check if a resource with this name exist
	template <typename T> static bool has(const String& name);

	// parse a library.json and load every resources in memory.
	static void parse(const Path& path);
	// serialize a library.json and save updated resources
	static void serialize(const Path& path);
private:
	static ResourceAllocator<Texture> textures;
	static ResourceAllocator<Mesh> meshes;
	static ResourceAllocator<AudioStream> audios;
	//static ResourceAllocator<Font> fonts;
	// TODO how to import big model with transform ?
	// 	   -> informations stored in scene. importation is a separated step.
	// TODO how to import cubemap ?
	//	   -> custom loader but no path to save.
};

template <> inline ResourceAllocator<Texture>& ResourceManager::allocator() { return textures; }
template <> inline ResourceAllocator<Mesh>& ResourceManager::allocator() { return meshes; }
template <> inline ResourceAllocator<AudioStream>& ResourceManager::allocator() { return audios; }

template <typename T>
inline std::shared_ptr<T> ResourceManager::get(const String& name)
{
	return allocator<T>().get(name);
}
template <typename T>
static Resource<T> ResourceManager::getResource(const String& name)
{
	return allocator<T>().getResource(name);
}
template <typename T>
static Resource<T> ResourceManager::load(const String& name, const Path& path)
{
	return allocator<T>().load(name, path);
}
template <typename T>
static Resource<T> ResourceManager::load(const String& name, T&& data)
{
	return allocator<T>().load(name, std::move(data));
}
template <typename T>
static void ResourceManager::unload(const String& name)
{
	return allocator<T>().unload(name);
}
template <typename T>
static size_t ResourceManager::count()
{
	return allocator<T>().count();
}
template <typename T>
static bool ResourceManager::has(const String& name)
{
	return allocator<T>().has(name);
}

};