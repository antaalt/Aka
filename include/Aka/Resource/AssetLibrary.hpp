#pragma once

#include <Aka/OS/OS.h>
#include <Aka/Core/Event.h>
#include <Aka/Resource/Resource/StaticMesh.hpp>
#include <Aka/Resource/Resource/Texture.hpp>
#include <Aka/Resource/Resource/Scene.hpp>
#include <Aka/Resource/Archive/ArchiveScene.hpp>

#include <map>

namespace aka {

template<typename T> struct ArchiveTrait { using Archive = Archive; };
template<> struct ArchiveTrait<SceneAvecUnNomChelou> { using Archive = ArchiveScene; };
template<> struct ArchiveTrait<StaticMesh> { using Archive = ArchiveStaticMesh; };
template<> struct ArchiveTrait<Texture> { using Archive = ArchiveImage; };

template <typename T>
class ResourceIterator
{
	using Iterator = typename std::map<ResourceID, ResourceHandle<T>>::iterator;
	static_assert(std::is_base_of<Resource, T>::value, "This should inherit Resource");
public:
	explicit ResourceIterator(Iterator& value) : m_value(value) {}
	ResourceIterator& operator++()
	{
		m_value++;
		return *this;
	}
	ResourceIterator operator++(int)
	{
		ResourceIterator old = *this;
		++(*this);
		return old;
	}
	std::pair<ResourceID, ResourceHandle<T>> operator*() { return *m_value; }
	bool operator==(const ResourceIterator<T>& value) { return value.m_value == m_value; }
	bool operator!=(const ResourceIterator<T>& value) { return value.m_value != m_value; }
private:
	Iterator m_value;
};

template <typename T>
class ResourceRange
{
	static_assert(std::is_base_of<Resource, T>::value, "This should inherit Resource");
public:
	ResourceRange(std::map<ResourceID, ResourceHandle<T>>& resources) : m_resources(resources) {}

	ResourceIterator<T> begin() { return ResourceIterator<T>(m_resources.begin()); }
	ResourceIterator<T> end() { return ResourceIterator<T>(m_resources.end()); }
private:
	std::map<ResourceID, ResourceHandle<T>>& m_resources;
};

struct AssetIterator {
private:
	using Iterator = std::map<AssetID, AssetInfo>::iterator;
public:
	explicit AssetIterator(Iterator& value) : m_value(value) {}
	AssetIterator& operator++()
	{
		m_value++;
		return *this;
	}
	AssetIterator operator++(int)
	{
		AssetIterator old = *this;
		++(*this);
		return old;
	}
	std::pair<AssetID, AssetInfo> operator*() { return *m_value; }
	bool operator==(const AssetIterator& value) { return value.m_value == m_value; }
	bool operator!=(const AssetIterator& value) { return value.m_value != m_value; }
private:
	Iterator m_value;
};

struct AssetRange {
	AssetRange(std::map<AssetID, AssetInfo>& assets) : m_assets(assets) {}

	AssetIterator begin() { return AssetIterator(m_assets.begin()); }
	AssetIterator end() { return AssetIterator(m_assets.end()); }
private:
	std::map<AssetID, AssetInfo>& m_assets;
};


ResourceType getResourceType(AssetType _type);

struct AssetAddedEvent
{
	AssetID asset;
};
struct ResourceLoadedEvent
{
	ResourceID resource;
};

class AssetLibrary
{
public:
	AssetLibrary();
	~AssetLibrary();
	// parse library.json & store AssetInfo with AssetID
	// Or parse whole folder looking for asset, with headers, we can id them
	void parse();
	// serialize library.json 
	void serialize();
public:
	AssetID registerAsset(const AssetPath& _path, AssetType _assetType);
	ResourceID getResourceID(AssetID _assetID) const;
	AssetID getAssetID(ResourceID _resourceID) const;
	AssetInfo getAssetInfo(AssetID _assetID);

public:
	template <typename T> ResourceHandle<T> get(ResourceID _resourceID);
	template <typename T> ResourceHandle<T> load(ResourceID _resourceID, gfx::GraphicDevice* _device);
	template <typename T> ResourceHandle<T> load(ResourceID _resourceID, const typename ArchiveTrait<T>::Archive& _archive, gfx::GraphicDevice* _device);
	template <typename T> ArchiveSaveResult save(ResourceID _resourceID, gfx::GraphicDevice* _device);
	template <typename T> void unload(ResourceID _resourceID, gfx::GraphicDevice* _device);
	
protected:
	friend class Application;
	// Update the library, handle streaming & unload useless assets
	void update();
	// Destroy all assets from library.
	void destroy(gfx::GraphicDevice* _device);
public:
	// Iterate a resource
	template<typename T> ResourceRange<T> getRange();
	// Iterate assets
	AssetRange getAssetRange() { return AssetRange(m_assets); }

private:
	template <typename T>
	using ResourceMap = std::map<ResourceID, ResourceHandle<T>>;
	template <typename T>
	ResourceMap<T>& getResourceMap();
private:
	std::map<AssetID, AssetInfo> m_assets;
	std::map<ResourceID, AssetID> m_resources;
private:
	ResourceMap<StaticMesh> m_staticMeshes;
	ResourceMap<SceneAvecUnNomChelou> m_scenes;
	ResourceMap<Texture> m_textures;
};

template<> AssetLibrary::ResourceMap<SceneAvecUnNomChelou>& AssetLibrary::getResourceMap();
template<> AssetLibrary::ResourceMap<StaticMesh>& AssetLibrary::getResourceMap();
template<> AssetLibrary::ResourceMap<Texture>& AssetLibrary::getResourceMap();

template<typename T>
ResourceRange<T> AssetLibrary::getRange()
{ 
	return ResourceRange<T>(getResourceMap<T>()); 
}

template<typename T>
ResourceHandle<T> AssetLibrary::get(ResourceID _resourceID)
{
	static_assert(std::is_base_of<Resource, T>::value, "Invalid resource type");
	ResourceMap<T>& map = getResourceMap<T>();
	auto itResource = map.find(_resourceID);
	if (itResource != map.end())
	{
		return itResource->second;
	}
	return ResourceHandle<T>::invalid();
}

template<typename T>
inline ResourceHandle<T> AssetLibrary::load(ResourceID _resourceID, gfx::GraphicDevice* _device)
{
	auto it = m_resources.find(_resourceID);
	if (it == m_resources.end())
		return ResourceHandle<T>::invalid();
	ArchiveTrait<T>::Archive archive(it->second);
	ArchiveLoadResult res = archive.load(ArchiveLoadContext(this));
	if (res != ArchiveLoadResult::Success)
		return ResourceHandle<T>::invalid();
	return load<T>(_resourceID, archive, _device);
}

template<typename T>
inline ResourceHandle<T> AssetLibrary::load(ResourceID _resourceID, const typename ArchiveTrait<T>::Archive& _archive, gfx::GraphicDevice* _device)
{
	static_assert(std::is_base_of<Resource, T>::value, "Invalid resource type");
	static_assert(std::is_base_of<Archive, ArchiveTrait<T>::Archive>::value, "Invalid archive type");
	ResourceMap<T>& map = getResourceMap<T>();
	// Check if resource already exist.
	auto itResource = map.find(_resourceID);
	if (itResource != map.end())
	{
		//Logger::warn("Trying to load a resource that is already loaded.");
		return itResource->second;
	}
	// Get assetID corresponding to resource.
	auto itAssetID = m_resources.find(_resourceID);
	if (itAssetID == m_resources.end())
	{
		return ResourceHandle<T>::invalid();
	}
	AssetID assetID = itAssetID->second;
	// Get assetInfo
	auto itAssetInfo = m_assets.find(assetID);
	if (itAssetInfo == m_assets.end())
	{
		return ResourceHandle<T>::invalid();
	}
	const AssetInfo& assetInfo = itAssetInfo->second;
	String name = OS::File::basename(assetInfo.path.getRawPath());
	auto it = map.insert(std::make_pair(_resourceID, ResourceHandle<T>(_resourceID, name)));
	if (it.second)
	{
		AKA_ASSERT(_device != nullptr, "Invalid device");
		ResourceHandle<T> handle = it.first->second;
		handle.get().create(this, _device, _archive);
		EventDispatcher<ResourceLoadedEvent>::emit(ResourceLoadedEvent{ _resourceID });
		return handle;
	}
	else
	{
		// If we reach this code, it means the resource does not exist & could not be added.
		AKA_UNREACHABLE;
		return ResourceHandle<T>::invalid();
	}
}

template<typename T>
inline ArchiveSaveResult AssetLibrary::save(ResourceID _resourceID, gfx::GraphicDevice* _device)
{
	auto it = m_resources.find(_resourceID);
	if (it == m_resources.end())
		return ArchiveSaveResult::Failed;
	ResourceMap<T>& map = getResourceMap<T>();
	auto itResource = map.find(_resourceID);
	if (itResource == map.end())
		return ArchiveSaveResult::Failed;
	if (!itResource->second.isLoaded())
		return ArchiveSaveResult::Failed;
	ArchiveTrait<T>::Archive archive(it->second);
	itResource->second.get().save(this, _device, archive);
	ArchiveSaveResult res = archive.save(ArchiveSaveContext(this));
	return res;
}

template<typename T>
inline void AssetLibrary::unload(ResourceID _resourceID, gfx::GraphicDevice* _device)
{
	ResourceMap<T>& map = getResourceMap<T>();
	auto itResource = map.find(_resourceID);
	if (itResource == map.end())
		return;
	if (!itResource->second.isLoaded())
		return; // resource not loaded
	itResource->second.destroy(this, _device);
}

}