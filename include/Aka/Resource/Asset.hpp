#pragma once

#include <Aka/Resource/AssetPath.hpp>

namespace aka {

enum class AssetID : uint64_t 
{
	Invalid = -1
};

enum class AssetType : uint32_t
{
	Unknown,

	Geometry,
	Material,
	Batch,
	StaticMesh,
	DynamicMesh,
	Image,
	Font,
	Audio,
	Scene,

	First = Geometry,
	Last = Scene,
};


const char* getAssetTypeString(AssetType type);

struct AssetCache
{
	size_t size; // size of asset.
	AssetType type; // Type could be cached ?
};

struct AssetInfo 
{
	AssetID id;
	AssetPath path;
	AssetType type;
	//AssetCache* cache;

	static AssetInfo invalid() {
		return AssetInfo{
			AssetID::Invalid,
			"",
			AssetType::Unknown
		};
	}
};


}