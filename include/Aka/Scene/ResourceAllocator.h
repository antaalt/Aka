#pragma once

#include <unordered_map>
#include <mutex>
#include <memory>

#include <Aka/OS/Logger.h>
#include <Aka/OS/FileSystem.h>
#include <Aka/Core/String.h>

#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Mesh.h>
#include <Aka/Audio/AudioStream.h>	

namespace std { template <> struct hash<aka::String> { size_t operator()(const aka::String& k) const { return hash<string>()(k); } }; }

namespace aka {

template<typename T>
struct Resource
{
	std::shared_ptr<T> resource;
	Path path;
	Time::Unit loaded; // When the resource was loaded
	Time::Unit updated; // When the resource was updated. TODO use event ?
	size_t size;

	// Load a resource from library folder
	static Resource<T> load(const Path& path);
	// Save a resource to library folder
	static void save(const Path& path, const Resource<T>& resource);
};


template<typename T>
class ResourceAllocator
{
	using map = std::unordered_map<String, Resource<T>>;
public:
	// Load a resource from the given path
	Resource<T> load(const String& name, const Path& path);
	// Load a resource from an object built internally
	Resource<T> load(const String& name, std::shared_ptr<T>&& data);
	
	// Unload a resource from the database
	void unload(const String& name);
	// Check if the name is registered
	bool has(const String& name) const;
	// Get the number of allocated resources
	size_t count() const;
	// Get a resource directly
	std::shared_ptr<T> get(const String& name);
	// Get a resource with its informations
	Resource<T>& getResource(const String& name);

	// Iterators
	typename map::iterator begin() { return m_resources.begin(); }
	typename map::iterator end() { return m_resources.end(); }
	typename map::const_iterator begin() const { return m_resources.begin(); }
	typename map::const_iterator end() const { return m_resources.end(); }
private:
	mutable std::mutex m_lock;
	std::unordered_map<String, Resource<T>> m_resources;
};

using TextureAllocator = ResourceAllocator<Texture>;
using AudioAllocator = ResourceAllocator<AudioStream>;
using MeshAllocator = ResourceAllocator<Mesh>;


template<typename T>
inline Resource<T> ResourceAllocator<T>::load(const String& name, const Path& path)
{
	std::lock_guard<std::mutex> m(m_lock);
	auto it = m_resources.find(name);
	if (it == m_resources.end())
	{
		Resource<T> res = Resource<T>::load(path);
		if (res.resource == nullptr)
		{
			Logger::error("Resource failed to load : ", name);
			return Resource<T>{};
		}
		else
		{
			auto it = m_resources.insert(std::make_pair(name, res));
			return it.first->second;
		}
	}
	else
	{
		Logger::error("Resource already loaded : ", name);
		return Resource<T>{};
	}
}

template<typename T>
inline Resource<T> ResourceAllocator<T>::load(const String& name, std::shared_ptr<T>&& data)
{
	std::lock_guard<std::mutex> m(m_lock);
	auto it = m_resources.find(name);
	if (it == m_resources.end())
	{
		Resource<T> res;
		res.loaded = Time::now();
		res.updated = Time::now();
		res.path = "";
		res.size = 0;
		res.resource = std::move(data);
		if (res.resource == nullptr)
		{
			Logger::error("Resource failed to load : ", name);
			return Resource<T>{};
		}
		else
		{
			auto it = m_resources.insert(std::make_pair(name, res));
			return it.first->second;
		}
	}
	else
	{
		Logger::error("Resource already loaded : ", name);
		return Resource<T>{};
	}
}

template<typename T>
inline void ResourceAllocator<T>::unload(const String& name)
{
	std::lock_guard<std::mutex> m(m_lock);
	auto it = m_resources.find(name);
	if (it != m_resources.end())
		m_resources.erase(it);
}

template<typename T>
inline bool ResourceAllocator<T>::has(const String& name) const
{
	std::lock_guard<std::mutex> m(m_lock);
	auto it = m_resources.find(name);
	return it != m_resources.end();
}

template<typename T>
inline size_t ResourceAllocator<T>::count() const
{
	std::lock_guard<std::mutex> m(m_lock);
	return m_resources.size();
}

template<typename T>
inline std::shared_ptr<T> ResourceAllocator<T>::get(const String& name)
{
	std::lock_guard<std::mutex> m(m_lock);
	auto it = m_resources.find(name);
	if (it == m_resources.end())
		return nullptr;
	return it->second.resource;
}

template<typename T>
inline Resource<T>& ResourceAllocator<T>::getResource(const String& name)
{
	std::lock_guard<std::mutex> m(m_lock);
	return m_resources.find(name)->second;
}

};