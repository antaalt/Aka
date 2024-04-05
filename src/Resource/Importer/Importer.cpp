#include <Aka/Resource/Importer/Importer.hpp>

namespace aka {

Importer::Importer(AssetLibrary* _library) :
	m_library(_library),
	m_path(AssetPath()),
	m_name(""),
	m_flags(ImportFlag::None)
{
}
Importer:: ~Importer()
{
}

void Importer::setFlag(ImportFlag flag)
{
	m_flags |= flag;
}
void Importer::setAssetPath(const AssetPath& path)
{
	m_path = path;
}
void Importer::setName(const String& name)
{
	m_name = name;
}
bool Importer::has(ImportFlag flag) const
{
	return asBool(flag & m_flags);
}
AssetPath Importer::getAssetPath(const char* subPath) const
{
	if (subPath == nullptr)
	{
		return m_path;
	}
	else
	{
		String str = m_path.cstr();
		str.append("/");
		str.append(subPath);
		return AssetPath(str, AssetPathType::Cooked);
	}
}
const String& Importer::getName() const
{
	return m_name;
}

AssetLibrary* Importer::getAssetLibrary() const
{
	return m_library;
}

AssetID Importer::registerAsset(AssetType type, const String& name)
{
	String finalName = name;
	if (finalName.empty())
		finalName = String::format("Material %s %p", getAssetTypeString(type), &name);
	AKA_ASSERT(!finalName.empty(), "Missing name");
	String path;
	path.append(Importer::getAssetTypeName(type));
	path.append("/");
	path.append(name);
	path.append(".");
	path.append(Importer::getAssetExtension(type));
	AssetID id = m_library->registerAsset(getAssetPath(path.cstr()), type);
	if (id == AssetID::Invalid)
	{
		// TODO handle overwrite
		return AssetID::Invalid;
	}
	else
	{
		return id;
	}
}

const char* Importer::getAssetTypeName(AssetType type)
{
	switch (type)
	{
	case AssetType::Geometry: return "geometries";
	case AssetType::Material: return "materials";
	case AssetType::Batch: return "batches";
	case AssetType::StaticMesh: return "static-meshes";
	case AssetType::SkeletalMesh: return "skeletal-meshes";
	case AssetType::Skeleton: return "skeleton";
	case AssetType::SkeletonAnimation: return "skeleton-animation";
	case AssetType::Image:return "images";
	case AssetType::Scene:return "scenes";
	default:
		return nullptr;
	}
}
const char* Importer::getAssetExtension(AssetType type)
{
	switch (type)
	{
	default:
		AKA_UNREACHABLE;
		return nullptr;
	case AssetType::Geometry: return "geo";
	case AssetType::Material: return "mat";
	case AssetType::Batch: return "bat";
	case AssetType::StaticMesh: return "stm";
	case AssetType::SkeletalMesh: return "skm";
	case AssetType::Skeleton: return "ske";
	case AssetType::SkeletonAnimation: return "ska";
	case AssetType::Image:return "img";
	case AssetType::Font:return "fnt";
	case AssetType::Audio:return "odio";
	case AssetType::Scene:return "sce";
	}
}

};