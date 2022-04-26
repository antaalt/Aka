#pragma once

#include <Aka/Resource/ResourceAllocator.h>

#include <Aka/Rendering/Mesh.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Core/Font.h>
#include <Aka/Audio/AudioStream.h>

namespace aka {

class ResourceManager
{
public:
	ResourceManager() {}
	virtual ~ResourceManager() {}

	// Get the allocator of the given resource
	template <typename T> ResourceAllocator<T>& allocator();
	// Get the raw resource
	template <typename T> T* get(const String& name);
	// Get the resource 
	template <typename T> Resource<T> getResource(const String& name);
	// Load a resource from file
	template <typename T> Resource<T> load(const String& name, const Path& path);
	// Load a resource internally
	template <typename T> Resource<T> load(const String& name, T* data);
	// Unload a resource
	template <typename T> void unload(const String& name);
	// Get the name of a resource
	template <typename T> String name(const T* data);
	// Get the number of resources
	template <typename T> size_t count();
	// Check if a resource with this name exist
	template <typename T> bool has(const String& name);

	// parse a library.json and load every resources in memory.
	void parse(const Path& path);
	// serialize a library.json and save updated resources
	void serialize(const Path& path);

	// Generate an asset path from a relative path.
	static Path path(const Path& path);
private:
	static Path assetPath;
	ResourceAllocator<gfx::Texture> textures;
	ResourceAllocator<Mesh> meshes;
	ResourceAllocator<AudioStream> audios;
	ResourceAllocator<Font> fonts;
	ResourceAllocator<gfx::Buffer> buffers;
};

template <> inline ResourceAllocator<gfx::Texture>& ResourceManager::allocator() { return textures; }
template <> inline ResourceAllocator<Mesh>& ResourceManager::allocator() { return meshes; }
template <> inline ResourceAllocator<AudioStream>& ResourceManager::allocator() { return audios; }
template <> inline ResourceAllocator<Font>& ResourceManager::allocator() { return fonts; }
template <> inline ResourceAllocator<gfx::Buffer>& ResourceManager::allocator() { return buffers; }

template <typename T>
inline T* ResourceManager::get(const String& name)
{
	return allocator<T>().get(name).get();
}
template <typename T>
inline Resource<T> ResourceManager::getResource(const String& name)
{
	return allocator<T>().getResource(name);
}
template <typename T>
inline Resource<T> ResourceManager::load(const String& name, const Path& path)
{
	return allocator<T>().load(name, path);
}
template <typename T>
inline Resource<T> ResourceManager::load(const String& name, T* data)
{
	return allocator<T>().load(name, data);
}
template <typename T>
inline void ResourceManager::unload(const String& name)
{
	return allocator<T>().unload(name);
}
template <typename T>
inline size_t ResourceManager::count()
{
	return allocator<T>().count();
}
template <typename T>
inline String ResourceManager::name(const T* data)
{
	return allocator<T>().name(data);
}
template <typename T>
inline bool ResourceManager::has(const String& name)
{
	return allocator<T>().has(name);
}

};