#include <Aka/Resource/AssetImporter.h>

#include <Aka/OS/OS.h>
#include <Aka/Resource/Importer/TextureImporter.h>
#include <Aka/Resource/Importer/SpriteImporter.h>
#include <Aka/Resource/Importer/FontImporter.h>

namespace aka {

std::shared_ptr<AssetImporter> AssetImporter::s_importers[EnumToIntegral(ResourceType::Count)]{
	std::make_shared<TextureImporter>(),
	nullptr,
	std::make_shared<SpriteImporter>(),
	nullptr,
	std::make_shared<FontImporter>(),
	nullptr,
	nullptr,
};

void AssetImporter::attach(ResourceType type, AssetImporter* importer)
{
	s_importers[EnumToIntegral(type)] = std::shared_ptr<AssetImporter>(importer);
}

bool AssetImporter::import(const Path& path, ResourceType type, std::function<void(Asset& asset)>&& callback)
{
	String assetName = OS::File::name(path);
	Path assetPath = OS::cwd() + Path("library/") + assetName; // TODO add extension & type folder.
	if (!OS::File::exist(path))
	{
		return false; // error
	}
	if (OS::File::exist(assetPath))
	{
		// TODO add flag to override ?
		//return false; // error, asset already exist.
	}
	AssetImporter* importer = s_importers[EnumToIntegral(type)].get();
	if (importer == nullptr)
	{
		Logger::error("Not importer for asset type : ", EnumToIntegral(type));
		return false; // Invalid importer
	}

	bool importation = importer->import(path, [&](Resource* resource) -> bool {
		AKA_ASSERT(type == resource->getType(), "Invalid resource type");
		std::unique_ptr<ResourceArchive> archive(resource->createResourceArchive());
		AKA_ASSERT(archive.get() != nullptr, "No archive found.");
		FileStream stream(assetPath, FileMode::Write, FileType::Binary);
		if (archive->save(stream, resource->getBuildData()))
		{
			stream.close();
			// If its saved, build the asset.
			Asset asset;
			asset.name = assetName;
			asset.path = assetPath;
			asset.type = type;
			asset.resource = resource;
			asset.diskSize = OS::File::size(assetPath);
			callback(asset);
			return true;
		}
		else
		{
			// Failed to import
			return false;
		}
	});
	return importation;
}

bool AssetImporter::import(const Path& path, ResourceType type)
{
	return AssetImporter::import(path, type, [](Asset&) {});
}

bool AssetImporter::importAndLoad(const Path& path, ResourceType type, gfx::GraphicDevice* device)
{
	return AssetImporter::import(path, type, [&](Asset& asset) { asset.load(device); });
}

};