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
	Result<Image> imgRes = ImageDecoder::fromDisk(path);
	if (imgRes.isErr())
		return ImportResult::CouldNotReadFile;

	ArchiveImage image(registerAsset(AssetType::Image, getName()));
	Image img = imgRes.getData();
	image.width = img.width;
	image.height = img.height;
	image.channels = img.getComponents();
	image.data = std::move(img.bytes);
	ArchiveParseResult res = image.save(ArchiveSaveContext(image, getAssetLibrary()));

	return (res == ArchiveParseResult::Success) ? ImportResult::Succeed : ImportResult::Failed;
}

ImportResult TextureImporter::import(const Blob & blob)
{
	Result<Image> imgRes = ImageDecoder::fromMemory(blob);
	if (imgRes.isErr())
		return ImportResult::CouldNotReadFile;
		
	ArchiveImage image(registerAsset(AssetType::Image, getName()));
	Image img = imgRes.getData();
	image.width = img.width;
	image.height = img.height;
	image.channels = img.getComponents();
	image.data = std::move(img.bytes);
	ArchiveParseResult res = image.save(ArchiveSaveContext(image, getAssetLibrary()));

	return (res == ArchiveParseResult::Success) ? ImportResult::Succeed : ImportResult::Failed;
}

};