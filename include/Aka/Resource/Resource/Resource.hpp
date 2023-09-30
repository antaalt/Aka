#pragma once 

#include <type_traits>
#include <memory>

#include <Aka/Core/Container/String.h>
#include <Aka/Resource/Asset.hpp>

namespace aka {

class Renderer;
class AssetLibrary;
struct Archive;
struct ArchiveLoadContext;
struct ArchiveSaveContext;

enum class ResourceType : uint32_t
{
	Unknown,

	Scene,
	StaticMesh,
	Material,
	Texture,
	Sprite,
	Font,
	AudioSource,

	First = Scene,
	Last = AudioSource,
};

const char* getResourceTypeString(ResourceType type);

enum class ResourceState {
	Unknown,
	Disk,
	Pending,
	Loaded,
};

class Resource
{
public:
	Resource(ResourceType _type);
	Resource(ResourceType _type, AssetID id, const String& _name);
	virtual ~Resource() {}

	const String& getName() const { return m_name; }
	AssetID getID() const { return m_id; }
	ResourceType getType() const { return m_type; }
	ResourceState getState() const { return m_state; }

	void fromArchive(ArchiveLoadContext& _context, Renderer* _renderer);
	void toArchive(ArchiveSaveContext& _context, Renderer* _renderer);
	void destroy(AssetLibrary* _library, Renderer* _renderer);
protected:
	virtual void fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer) = 0;
	virtual void toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer) = 0;
	virtual void destroy_internal(AssetLibrary* _library, Renderer* _renderer) = 0;
private:
	ResourceState m_state;
	ResourceType m_type;
	AssetID m_id;
	String m_name;
};


template <typename T>
struct ResourceHandle {
	static_assert(std::is_base_of<Resource, T>::value, "Type should inherit Resource");
public:
	ResourceHandle();
	ResourceHandle(AssetID _id, const String& _name);

	bool isValid() const;
	bool isLoaded() const;
	ResourceState getState() const;
	size_t getCount() const;
	void reset();

	const T& get() const;
	T& get();

public:
	static ResourceHandle<T> invalid();
private:
	std::shared_ptr<T> m_resource;
};

template<typename T>
inline ResourceHandle<T>::ResourceHandle() : 
	m_resource(nullptr)
{
}

template<typename T>
inline ResourceHandle<T>::ResourceHandle(AssetID _id, const String& _name) :
	m_resource(std::make_shared<T>(_id, _name))
{
}

template<typename T>
inline bool ResourceHandle<T>::isValid() const
{
	return m_resource != nullptr;
}

template<typename T>
inline bool ResourceHandle<T>::isLoaded() const
{
	return isValid() && m_resource->getState() == ResourceState::Loaded;
}

template<typename T>
inline ResourceState ResourceHandle<T>::getState() const
{
	if (!isValid())
		return ResourceState::Unknown;
	return m_resource->getState();
}

template<typename T>
inline size_t ResourceHandle<T>::getCount() const
{
	return m_resource.use_count();
}

template<typename T>
inline void ResourceHandle<T>::reset()
{
	m_resource.reset();
}

template<typename T>
inline const T& ResourceHandle<T>::get() const
{
	AKA_ASSERT(isValid(), "Accessing not valid resource");
	return *m_resource;
}
template<typename T>
inline T& ResourceHandle<T>::get()
{
	AKA_ASSERT(isValid(), "Accessing not valid resource");
	return *m_resource;
}

template<typename T>
inline ResourceHandle<T> ResourceHandle<T>::invalid()
{
	return ResourceHandle<T>();
}

}