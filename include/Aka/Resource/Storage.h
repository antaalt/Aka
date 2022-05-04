#pragma once

#include <Aka/OS/Path.h>

namespace aka {

// Binary format API :
// library.json is simply a list of name to a path.
// format are .mesh, .tex, .audio, .font, .anim
// .mesh : header, mesh attributes, has indices, vertex buffers, index buffer.
// .tex : header, texture type, texture(s) in png (if cubemap, 6 textures, else 1), use compression KTX
// .audio : header, raw mp3 file (or use ogg ?)
// .font : header, raw ttf file (or texture atlas with info about every character ? Avoid to be dependent on freetype)
// .buffer : header, buffer info, raw buffer
// .anim :

template <typename T>
struct IStorage
{
	// TODO use stream instead
	// Load a resource from a path
	virtual bool load(const Path& path) = 0;
	// Save a resource to a path
	virtual bool save(const Path& path) const = 0;
	// Allocate the resource on GPU
	virtual T* allocate() const = 0;
	// Deallocate the resource from GPU
	virtual void deallocate(T* resource) const = 0;
	// Convert the resource to a storage
	virtual void serialize(const T& resource) = 0;
	// Get the size of a resource
	virtual size_t size(const T& resource) = 0;
	// Create the storage from type
	static std::unique_ptr<IStorage<T>> create();
};
}; // namespace aka