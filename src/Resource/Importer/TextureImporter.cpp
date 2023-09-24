#include <Aka/Resource/Importer/TextureImporter.hpp>

#include <Aka/OS/Image.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Resource/Resource/Texture.hpp>
#include <Aka/Resource/Archive/ArchiveImage.hpp>
#include <Aka/OS/OS.h>

#include <stb_image.h>

namespace aka {

ImportResult TextureImporter::import(AssetLibrary * _library, const Path & path)
{
	// TODO handle HDR / ENVMAP / CUBEMAP / ARRAY
	Image img = ImageDecoder::fromDisk(path);
	if (img.size() == 0)
		return ImportResult::CouldNotReadFile;
	
	AssetPath assetPath = getAssetPath((OS::File::basename(path) + ".img").cstr());

	ArchiveImage image(_library->registerAsset(assetPath, AssetType::Image));
	image.width = img.width;
	image.height = img.height;
	image.channels = img.getComponents();
	image.data = std::move(img.bytes);
	ArchiveSaveResult res = image.save(ArchiveSaveContext(_library));

	return (res == ArchiveSaveResult::Success) ? ImportResult::Succeed : ImportResult::Failed;
}

ImportResult TextureImporter::import(AssetLibrary * _library, const Blob & blob)
{
	Image img = ImageDecoder::fromMemory(blob);
	if (img.size() == 0)
		return ImportResult::CouldNotReadFile;

	// TODO find unused name.
	AssetPath assetPath = getAssetPath("memory.img");
	
	ArchiveImage image(_library->registerAsset(assetPath, AssetType::Image));
	image.width = img.width;
	image.height = img.height;
	image.channels = img.getComponents();
	image.data = std::move(img.bytes);
	ArchiveSaveResult res = image.save(ArchiveSaveContext(_library));

	return (res == ArchiveSaveResult::Success) ? ImportResult::Succeed : ImportResult::Failed;
}

};