#pragma once

#include <Aka/OS/OS.h>
#include <Aka/Core/Event.h>
#include <Aka/Resource/Resource/StaticMesh.hpp>
#include <Aka/Resource/Resource/SkeletalMesh.hpp>
#include <Aka/Resource/Resource/Texture.hpp>
#include <Aka/Resource/Resource/Scene.hpp>
#include <Aka/Resource/Resource/Material.hpp>
#include <Aka/Resource/Archive/ArchiveScene.hpp>
#include <Aka/Resource/Archive/ArchiveStaticMesh.hpp>
#include <Aka/Resource/Archive/ArchiveSkeletalMesh.hpp>
#include <Aka/Resource/Archive/ArchiveImage.hpp>
#include <Aka/Resource/Archive/ArchiveMaterial.hpp>

#include <map>

namespace aka {

template<typename T> struct ArchiveTrait { using Archive = Archive; };
template<> struct ArchiveTrait<Scene> { using Archive = ArchiveScene; };
template<> struct ArchiveTrait<StaticMesh> { using Archive = ArchiveStaticMesh; };
template<> struct ArchiveTrait<SkeletalMesh> { using Archive = ArchiveSkeletalMesh; };
template<> struct ArchiveTrait<Texture> { using Archive = ArchiveImage; };
template<> struct ArchiveTrait<Material> { using Archive = ArchiveMaterial; };

template <typename T>
class ResourceIterator
{
	using Iterator = typename HashMap<AssetID, ResourceHandle<T>>::iterator;
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
	std::pair<AssetID, ResourceHandle<T>> operator*() { return *m_value; }
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
	ResourceRange(HashMap<AssetID, ResourceHandle<T>>& resources) : m_resources(resources) {}

	ResourceIterator<T> begin() { return ResourceIterator<T>(m_resources.begin()); }
	ResourceIterator<T> end() { return ResourceIterator<T>(m_resources.end()); }
private:
	HashMap<AssetID, ResourceHandle<T>>& m_resources;
};

struct AssetIterator {
private:
	using Iterator = HashMap<AssetID, AssetInfo>::iterator;
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
	AssetRange(HashMap<AssetID, AssetInfo>& assets) : m_assetInfo(assets) {}

	AssetIterator begin() { return AssetIterator(m_assetInfo.begin()); }
	AssetIterator end() { return AssetIterator(m_assetInfo.end()); }
private:
	HashMap<AssetID, AssetInfo>& m_assetInfo;
};


ResourceType getResourceType(AssetType _type);

struct AssetAddedEvent
{
	AssetID asset;
};
struct ResourceLoadedEvent
{
	AssetID resource;
	ResourceType type;
};

class AssetLibrary
{
public:
	AssetLibrary();
	AssetLibrary(const AssetLibrary&) = delete;
	AssetLibrary(AssetLibrary&&) = delete;
	AssetLibrary& operator=(const AssetLibrary&) = delete;
	AssetLibrary& operator=(AssetLibrary&&) = delete;
	~AssetLibrary();
	// parse library.json & store AssetInfo with AssetID
	// Or parse whole folder looking for asset, with headers, we can id them
	void parse();
	// serialize library.json 
	void serialize();
public:
	AssetID registerAsset(const AssetPath& _path, AssetType _assetType, bool _overwrite = false);
	AssetInfo getAssetInfo(AssetID _assetID);

public:
	template <typename T> ResourceHandle<T> get(AssetID _assetID);
	template <typename T> ResourceHandle<T> load(AssetID _assetID, Renderer* _renderer);
	template <typename T> ResourceHandle<T> load(AssetID _assetID, ArchiveLoadContext& _context, Renderer* _renderer);
	template <typename T> ArchiveParseResult save(AssetID _assetID, ArchiveSaveContext& _context, Renderer* _renderer);
	template <typename T> void unload(AssetID _assetID, Renderer* _renderer);
	template <typename T> bool isLoaded(AssetID _assetID);
	template <typename T> ResourceState getState(AssetID _assetID);
protected:
	friend class Application;
	// Update the library, handle streaming & unload useless assets
	void update();
	// Destroy all assets from library.
	void destroy(Renderer* _renderer);
public:
	// Iterate a resource
	template<typename T> ResourceRange<T> getRange();
	// Iterate assets
	AssetRange getAssetRange() { return AssetRange(m_assetInfo); }

private:
	template <typename T>
	using ResourceMap = HashMap<AssetID, ResourceHandle<T>>;
	template <typename T>
	ResourceMap<T>& getResourceMap();
private:
	HashMap<AssetID, AssetInfo> m_assetInfo;
private:
	ResourceMap<StaticMesh> m_staticMeshes;
	ResourceMap<SkeletalMesh> m_skeletalMeshes;
	ResourceMap<Scene> m_scenes;
	ResourceMap<Texture> m_textures;
	ResourceMap<Material> m_materials;
};

template<> AssetLibrary::ResourceMap<Scene>& AssetLibrary::getResourceMap();
template<> AssetLibrary::ResourceMap<StaticMesh>& AssetLibrary::getResourceMap();
template<> AssetLibrary::ResourceMap<SkeletalMesh>& AssetLibrary::getResourceMap();
template<> AssetLibrary::ResourceMap<Texture>& AssetLibrary::getResourceMap();
template<> AssetLibrary::ResourceMap<Material>& AssetLibrary::getResourceMap();

template<typename T>
ResourceRange<T> AssetLibrary::getRange()
{ 
	return ResourceRange<T>(getResourceMap<T>()); 
}

template<typename T>
ResourceHandle<T> AssetLibrary::get(AssetID _assetID)
{
	static_assert(std::is_base_of<Resource, T>::value, "Invalid resource type");
	ResourceMap<T>& map = getResourceMap<T>();
	auto itResource = map.find(_assetID);
	if (itResource != map.end())
	{
		return itResource->second;
	}
	return ResourceHandle<T>::invalid();
}

template<typename T>
inline ResourceHandle<T> AssetLibrary::load(AssetID _assetID, Renderer* _renderer)
{
	ArchiveTrait<T>::Archive archive(_assetID);
	ArchiveLoadContext ctx(archive, this);
	ArchiveParseResult res = archive.load(ctx);
	if (res != ArchiveParseResult::Success)
	{
		String str = String::format("Failed to load asset because of following error:  %s", getArchiveParseResultString(res));
		AlertModal(AlertModalType::Error, "Failed to load asset", str.cstr());
		return ResourceHandle<T>::invalid();
	}
	return load<T>(_assetID, ctx, _renderer);
}

template<typename T>
inline ResourceHandle<T> AssetLibrary::load(AssetID _assetID, ArchiveLoadContext& _context, Renderer* _renderer)
{
	static_assert(std::is_base_of<Resource, T>::value, "Invalid resource type");
	static_assert(std::is_base_of<Archive, ArchiveTrait<T>::Archive>::value, "Invalid archive type");
	// Get assetInfo
	auto itAssetInfo = m_assetInfo.find(_assetID);
	AKA_ASSERT(itAssetInfo != m_assetInfo.end(), "Trying to load an asset that does not exist in library.");
	const AssetInfo& assetInfo = itAssetInfo->second;
	ResourceMap<T>& map = getResourceMap<T>();
	// Check if resource already loaded.
	auto itResource = map.find(_assetID);
	if (itResource != map.end() && itResource->second.isLoaded())
	{
		Logger::warn("Asset '", assetInfo.path.cstr(), "' already loaded.");
		return itResource->second;
	}
	String name = OS::File::basename(assetInfo.path.getRawPath());
	auto it = map.insert(std::make_pair(_assetID, ResourceHandle<T>(_assetID, name)));
	if (it.second)
	{
		ResourceHandle<T> handle = it.first->second;
		T& res = handle.get();
		res.fromArchive(_context, _renderer);
		EventDispatcher<ResourceLoadedEvent>::emit(ResourceLoadedEvent{ _assetID, res.getType() });
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
inline ArchiveParseResult AssetLibrary::save(AssetID _assetID, ArchiveSaveContext& _context, Renderer* _renderer)
{
	ResourceMap<T>& map = getResourceMap<T>();
	auto itResource = map.find(_assetID);
	if (itResource == map.end())
		return ArchiveParseResult::Failed;
	if (!itResource->second.isLoaded())
		return ArchiveParseResult::Failed;
	ArchiveTrait<T>::Archive archive(_assetID);
	itResource->second.get().toArchive(_context, _renderer);
	ArchiveParseResult res = archive.save(_context);
	return res;
}

template<typename T>
inline void AssetLibrary::unload(AssetID _assetID, Renderer* _renderer)
{
	ResourceMap<T>& map = getResourceMap<T>();
	auto itResource = map.find(_assetID);
	if (itResource == map.end())
		return;
	if (!itResource->second.isLoaded())
		return; // resource not loaded
	itResource->second.get().destroy(this, _renderer);
	map.erase(itResource);
}

template<typename T>
inline bool AssetLibrary::isLoaded(AssetID _assetID)
{
	ResourceMap<T>& map = getResourceMap<T>();
	return 1 == map.count(_assetID);
}

template<typename T>
inline ResourceState AssetLibrary::getState(AssetID _assetID)
{
	ResourceMap<T>& map = getResourceMap<T>();
	auto it = map.find(_assetID);
	if (it == map.end())
		return ResourceState::Disk;
	return it->second.getState();
}

}