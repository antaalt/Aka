#include <Aka/Resource/AssetLibrary.hpp>

#include <Aka/OS/OS.h>

#include <nlohmann/json.hpp>
#include <fstream>

namespace aka {

ResourceID generateResourceIDFromAssetID(AssetID id)
{
	return ResourceID(id);
}

AssetID generateAssetIDFromAssetPath(const AssetPath& path)
{
	// With an AssetID depending on path, moving this asset will break all references...
	size_t hash = aka::hash(path.cstr(), path.size());
	return AssetID(hash);
}

ResourceType getResourceType(AssetType _type)
{
	switch (_type)
	{
	default:
	case AssetType::Batch:
	case AssetType::Material:
	case AssetType::Geometry:
	case AssetType::DynamicMesh:
	case AssetType::Font:
	case AssetType::Audio:
		return ResourceType::Unknown;
	case AssetType::Image:
		return ResourceType::Texture;
	case AssetType::StaticMesh:
		return ResourceType::StaticMesh;
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

	Path path = AssetPath::getAssetPath() + "library.json";
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
			AssetID id = registerAsset(AssetPath(path.c_str()), type);
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
		for (auto& pair : m_assets)
		{
			json dataAsset = json::object();
			dataAsset["id"] = (uint64_t)pair.first;
			dataAsset["path"] = pair.second.path.cstr();
			dataAsset["type"] = (uint32_t)pair.second.type;
			dataAssets.push_back(dataAsset);
		}
		data["assets"] = dataAssets;
	}

	Path path = AssetPath::getAssetPath() + "library.json";
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
	case AssetType::DynamicMesh: return false;
	case AssetType::Image: return ArchiveImage(id).validate(_library);
	case AssetType::Font: return false;
	case AssetType::Audio: return false;
	case AssetType::Scene: return ArchiveScene(id).validate(_library);
	default:
	case AssetType::Unknown:
		return false;
	}
}

AssetID AssetLibrary::registerAsset(const AssetPath& _path, AssetType _assetType)
{
	AssetID assetID = generateAssetIDFromAssetPath(_path);
	auto itAsset = m_assets.insert(std::make_pair(assetID, AssetInfo{ assetID, _path, _assetType }));
	if (!itAsset.second)
	{
		// Check if the file already exist & is valid, if so, use it.
		bool samePath = itAsset.first->second.path == _path;
		bool fileExist = OS::File::exist(_path.getAbsolutePath());
		if (samePath && (!fileExist || validate(this, assetID, _assetType)))
			return assetID;
		else
			return AssetID::Invalid; // Avoid overwriting an asset. There might be hash conflict.
	}

	auto itAsset2 = m_assets.insert(std::make_pair(assetID, AssetInfo{ assetID, _path, _assetType }));
	ResourceID resourceID = generateResourceIDFromAssetID(assetID);

	ResourceType resourceType = getResourceType(_assetType);
	if (resourceType != ResourceType::Unknown)
	{
		auto itResource = m_resources.insert(std::make_pair(resourceID, assetID));
		if (!itResource.second)
			return AssetID::Invalid;
	}
	// Could check file if correct type but might not be created yet...
	EventDispatcher<AssetAddedEvent>::emit(AssetAddedEvent{ assetID });

	return assetID;
}

ResourceID AssetLibrary::getResourceID(AssetID _assetID) const
{
	return generateResourceIDFromAssetID(_assetID);
}

AssetID AssetLibrary::getAssetID(ResourceID _resourceID) const
{
	auto it = m_resources.find(_resourceID);
	if (it == m_resources.end())
		return AssetID::Invalid;
	return it->second;
}

AssetInfo AssetLibrary::getAssetInfo(AssetID _id)
{
	auto it = m_assets.find(_id);
	if (it == m_assets.end())
		return AssetInfo::invalid();
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
AssetLibrary::ResourceMap<Texture>& AssetLibrary::getResourceMap()
{
	return m_textures;
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
	for (auto it : m_scenes)
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
	for (auto it : m_textures)
	{
		if (it.second.isLoaded())
		{
			it.second.get().destroy(this, _renderer);
		}
	}
}

}