#pragma once

#include <Aka/Resource/ResourceAllocator.h>

#include <Aka/Graphic/Mesh.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Buffer.h>
#include <Aka/Core/Font.h>
#include <Aka/Audio/AudioStream.h>

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
	template <typename T> static Resource<T> load(const String& name, const std::shared_ptr<T>& data);
	// Unload a resource
	template <typename T> static void unload(const String& name);
	// Get the name of a resource
	template <typename T> static String name(const std::shared_ptr<T>& data);
	// Get the number of resources
	template <typename T> static size_t count();
	// Check if a resource with this name exist
	template <typename T> static bool has(const String& name);

	// parse a library.json and load every resources in memory.
	static void parse(const Path& path);
	// serialize a library.json and save updated resources
	static void serialize(const Path& path);

	// Generate an asset path from a relative path.
	static Path path(const Path& path);
private:
	static Path assetPath;
	static ResourceAllocator<Texture> textures;
	static ResourceAllocator<Mesh> meshes;
	static ResourceAllocator<AudioStream> audios;
	static ResourceAllocator<Font> fonts;
	static ResourceAllocator<Buffer> buffers;
};

template <> inline ResourceAllocator<Texture>& ResourceManager::allocator() { return textures; }
template <> inline ResourceAllocator<Mesh>& ResourceManager::allocator() { return meshes; }
template <> inline ResourceAllocator<AudioStream>& ResourceManager::allocator() { return audios; }
template <> inline ResourceAllocator<Font>& ResourceManager::allocator() { return fonts; }
template <> inline ResourceAllocator<Buffer>& ResourceManager::allocator() { return buffers; }

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
static Resource<T> ResourceManager::load(const String& name, const std::shared_ptr<T>& data)
{
	return allocator<T>().load(name, data);
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
static String ResourceManager::name(const std::shared_ptr<T>& data)
{
	return allocator<T>().name(data);
}
template <typename T>
static bool ResourceManager::has(const String& name)
{
	return allocator<T>().has(name);
}

};