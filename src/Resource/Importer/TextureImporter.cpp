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

TextureImporter::TextureImporter(AssetLibrary* _library) :
	Importer(_library)
{
}
ImportResult TextureImporter::import(const Path & path)
{
	// TODO handle HDR / ENVMAP / CUBEMAP / ARRAY
	Image img = ImageDecoder::fromDisk(path);
	if (img.size() == 0)
		return ImportResult::CouldNotReadFile;

	ArchiveImage image(registerAsset(AssetType::Image, getName()));
	image.width = img.width;
	image.height = img.height;
	image.channels = img.getComponents();
	image.data = std::move(img.bytes);
	ArchiveParseResult res = image.save(ArchiveSaveContext(image, getAssetLibrary()));

	return (res == ArchiveParseResult::Success) ? ImportResult::Succeed : ImportResult::Failed;
}

ImportResult TextureImporter::import(const Blob & blob)
{
	Image img = ImageDecoder::fromMemory(blob);
	if (img.size() == 0)
		return ImportResult::CouldNotReadFile;
		
	ArchiveImage image(registerAsset(AssetType::Image, getName()));
	image.width = img.width;
	image.height = img.height;
	image.channels = img.getComponents();
	image.data = std::move(img.bytes);
	ArchiveParseResult res = image.save(ArchiveSaveContext(image, getAssetLibrary()));

	return (res == ArchiveParseResult::Success) ? ImportResult::Succeed : ImportResult::Failed;
}

};