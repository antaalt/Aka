#include <Aka/Resource/Asset.hpp>

namespace aka {

const char* getAssetTypeString(AssetType type)
{
	switch (type)
	{
	case AssetType::Unknown: return "Unknown";
	case AssetType::Geometry: return "Geometry";
	case AssetType::Material: return "Material";
	case AssetType::Batch: return "Batch";
	case AssetType::StaticMesh: return "StaticMesh";
	case AssetType::DynamicMesh: return "DynamicMesh";
	case AssetType::Image:return "Image";
	case AssetType::Font:return "Font";
	case AssetType::Audio:return "Audio";
	case AssetType::Scene:return "Scene";
	default:
		AKA_ASSERT(false, "Invalid asset type");
		return nullptr;
	}
}
}