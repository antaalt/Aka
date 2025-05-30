#include <Aka/Resource/AssetLibrary.hpp>

#include <Aka/OS/OS.h>

#include <nlohmann/json.hpp>
#include <fstream>

#include <Aka/Resource/Archive/ArchiveGeometry.hpp>
#include <Aka/Resource/Archive/ArchiveBatch.hpp>
#include <Aka/Resource/Archive/ArchiveSkeleton.hpp>
#include <Aka/Resource/Archive/ArchiveSkeletonAnimation.hpp>

namespace aka {

AssetID generateAssetIDFromAssetPath(const AssetPath& path)
{
	// With an AssetID depending on path, moving this asset will break all references...
	return AssetID(aka::hash::fnv(path.cstr(), path.size()));
}

ResourceType getResourceType(AssetType _type)
{
	switch (_type)
	{
	default:
	case AssetType::Batch:
	case AssetType::Geometry:
	case AssetType::Font:
	case AssetType::Audio:
		return ResourceType::Unknown;
	case AssetType::Image:
		return ResourceType::Texture;
	case AssetType::StaticMesh:
		return ResourceType::StaticMesh;
	case AssetType::SkeletalMesh:
		return ResourceType::SkeletalMesh;
	case AssetType::Material:
		return ResourceType::Material;
	case AssetType::Scene:
		return ResourceType::Scene;
	}
	return ResourceType::Unknown;
}

AssetLibrary::AssetLibrary()
{
}
AssetLibrary::~AssetLibrary()
{
}

void AssetLibrary::parse()
{
	using json = nlohmann::json;

	Path path = AssetPath("library.json", AssetPathType::Cooked).getAbsolutePath();
	if (!OS::File::exist(path))
	{
		return;
	}
	std::ifstream f(path.cstr());

	json data = json::parse(f);

	{
		json& dataMetadata = data["metadata"];
	}

	{
		json& dataAssets = data["assets"];
		for (json& dataAsset : dataAssets)
		{
			uint64_t assetID = dataAsset["id"].get<uint64_t>();
			std::string path = dataAsset["path"].get<std::string>();
			AssetType type = dataAsset["type"].get<AssetType>();
			AssetID id = registerAsset(AssetPath(path.c_str(), AssetPathType::Cooked), type);
			AKA_ASSERT(id == AssetID(assetID), "Invalid assetID");
		}
	}
}

void AssetLibrary::serialize()
{
	using json = nlohmann::json;

	json data;

	{
		json dataMetadata = json::object();
		data["metadata"] = dataMetadata;
	}
	{
		json dataAssets = json::array();
		for (auto& pair : m_assetInfo)
		{
			json dataAsset = json::object();
			dataAsset["id"] = (uint64_t)pair.first;
			dataAsset["path"] = pair.second.path.cstr();
			dataAsset["type"] = (uint32_t)pair.second.type;
			dataAssets.push_back(dataAsset);
		}
		data["assets"] = dataAssets;
	}

	Path path = AssetPath("library.json", AssetPathType::Cooked).getAbsolutePath();
	std::ofstream f(path.cstr());
	f << data.dump(4);
}

bool validate(AssetLibrary* _library, AssetID id, AssetType _type)
{
	switch (_type)
	{
	case AssetType::Geometry: return ArchiveGeometry(id).validate(_library);
	case AssetType::Material: return ArchiveMaterial(id).validate(_library);
	case AssetType::Batch: return ArchiveBatch(id).validate(_library);
	case AssetType::StaticMesh: return ArchiveStaticMesh(id).validate(_library);
	case AssetType::SkeletalMesh: return ArchiveSkeletalMesh(id).validate(_library);
	case AssetType::Skeleton: return ArchiveSkeleton(id).validate(_library);
	case AssetType::SkeletonAnimation: return ArchiveSkeletonAnimation(id).validate(_library);
	case AssetType::Image: return ArchiveImage(id).validate(_library);
	case AssetType::Font: return false;
	case AssetType::Audio: return false;
	case AssetType::Scene: return ArchiveScene(id).validate(_library);
	default:
	case AssetType::Unknown:
		return false;
	}
}

AssetID AssetLibrary::registerAsset(const AssetPath& _path, AssetType _assetType, bool _overwrite)
{
	AssetID assetID = generateAssetIDFromAssetPath(_path);
	auto itAsset = m_assetInfo.insert(std::make_pair(assetID, AssetInfo{ assetID, _path, _assetType }));
	if (!itAsset.second)
	{
		// Check if the file already exist & is valid, if so, use it.
		bool samePath = itAsset.first->second.path == _path;
		bool sameType = itAsset.first->second.type == _assetType;
		bool fileExist = OS::File::exist(_path.getAbsolutePath());
		// Should check hash aswell to validate content.
		if (samePath && sameType && (!fileExist || validate(this, assetID, _assetType)) && !_overwrite)
		{
			//Logger::warn("[register] Asset '", _path.cstr(), "' already exist but is identical. Using it.");
			return assetID;
		}
		else if (_overwrite)
		{
			Logger::warn("[register] Asset '", _path.cstr(), "' overwritten.");
			m_assetInfo[assetID] = AssetInfo{ assetID, _path, _assetType };
		}
		else
		{
			Logger::error("[register] Asset '", _path.cstr(), "' already exist.");
			return AssetID::Invalid; // There might be hash conflict, or trying to overwrite old version.
		}
	}

	// Could check file if correct type but might not be created yet...
	EventDispatcher<AssetAddedEvent>::emit(AssetAddedEvent{ assetID });

	return assetID;
}

AssetInfo AssetLibrary::getAssetInfo(AssetID _id)
{
	auto it = m_assetInfo.find(_id);
	if (it == m_assetInfo.end())
	{
		Logger::error("[register] Asset '", (uint64_t)_id, "' could not be found.");
		return AssetInfo::invalid();
	}
	return it->second;
}

template<> 
AssetLibrary::ResourceMap<Scene>& AssetLibrary::getResourceMap()
{ 
	return m_scenes; 
}
template<> 
AssetLibrary::ResourceMap<StaticMesh>& AssetLibrary::getResourceMap()
{
	return m_staticMeshes;
}
template<>
AssetLibrary::ResourceMap<SkeletalMesh>& AssetLibrary::getResourceMap()
{
	return m_skeletalMeshes;
}
template<> 
AssetLibrary::ResourceMap<Texture>& AssetLibrary::getResourceMap()
{
	return m_textures;
}
template<>
AssetLibrary::ResourceMap<Material>& AssetLibrary::getResourceMap()
{
	return m_materials;
}

void AssetLibrary::update()
{
	EventDispatcher<AssetAddedEvent>::dispatch();
	EventDispatcher<ResourceLoadedEvent>::dispatch();
	// TODO check which assets should be unload by ref count & stream
	// Also run the asset indexer here, browse all files & add them to asset library.
	// Save library in temp ? or local temp folder.
	// Each asset has 
}

void AssetLibrary::destroy(Renderer* _renderer)
{
	for (auto it : m_textures)
	{
		if (it.second.isLoaded())
		{
			it.second.get().destroy(this, _renderer);
		}
	}
	for (auto it : m_staticMeshes)
	{
		if (it.second.isLoaded())
		{
			it.second.get().destroy(this, _renderer);
		}
	}
	for (auto it : m_skeletalMeshes)
	{
		if (it.second.isLoaded())
		{
			it.second.get().destroy(this, _renderer);
		}
	}
	for (auto it : m_scenes)
	{
		if (it.second.isLoaded())
		{
			it.second.get().destroy(this, _renderer);
		}
	}
	for (auto it : m_materials)
	{
		if (it.second.isLoaded())
		{
			it.second.get().destroy(this, _renderer);
		}
	}
}

}