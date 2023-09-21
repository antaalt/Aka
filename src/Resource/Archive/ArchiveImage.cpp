#include <Aka/Resource/Archive/ArchiveImage.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveImage::ArchiveImage() :
	ArchiveImage(AssetID::Invalid)
{
}
ArchiveImage::ArchiveImage(const AssetID& id) : 
	Archive(AssetType::Image, id, getLatestVersion()),
	data{},
	width(0),
	height(0),
	channels(0)
{
}
// This hugely improve loading time, but we lose in memory.
// Should store in compressed texture format instead (KTX & DDS)
// Could also have a flag that is telling if the image is compressed or not depending on the size on disk ?
//#define STORE_COMPRESSED_IMAGE 1
ArchiveLoadResult ArchiveImage::load_internal(ArchiveLoadContext& _context, BinaryArchive& _archive)
{
	this->width = _archive.read<uint32_t>();
	this->height = _archive.read<uint32_t>();
	this->channels = _archive.read<uint32_t>();

#if !defined(STORE_COMPRESSED_IMAGE)
	uint32_t size = _archive.read<uint32_t>();
	this->data.resize(size);
	_archive.read(this->data.data(), size);
#else
	uint32_t compressedSize = _archive.read<uint32_t>();
	Vector<uint8_t> data(compressedSize);
	_archive.read(data.data(), compressedSize);

	Image image;
	if (!image.decode(data.data(), data.size()))
		return ArchiveLoadResult::Failed;

	this->data.resize(image.size());
	Memory::copy(this->data.data(), image.data(), image.size());
#endif
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveImage::save_internal(ArchiveSaveContext& _context, BinaryArchive& _archive)
{
	_archive.write<uint32_t>(this->width);
	_archive.write<uint32_t>(this->height);
	_archive.write<uint32_t>(this->channels);
#if !defined(STORE_COMPRESSED_IMAGE)
	_archive.write<uint32_t>((uint32_t)this->data.size());
	_archive.write(this->data.data(), this->data.size());
#else

	Image image(this->width, this->height, this->channels, this->data.data());
	std::vector<uint8_t> encodedData = image.encodePNG();
	if (encodedData.empty())
		return ArchiveSaveResult::Failed;

	_archive.write<uint32_t>((uint32_t)encodedData.size());
	_archive.write(encodedData.data(), encodedData.size());
#endif

	return ArchiveSaveResult::Success;
}

ArchiveLoadResult ArchiveImage::load_dependency(ArchiveLoadContext& _context)
{
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveImage::save_dependency(ArchiveSaveContext& _context)
{
	return ArchiveSaveResult::Success;
}
void ArchiveImage::copyFrom(const Archive* _archive)
{
	AKA_ASSERT(_archive->id() == id(), "Invalid id");
	AKA_ASSERT(_archive->type() == type(), "Invalid type");
	AKA_ASSERT(_archive->version() == version(), "Invalid version");

	const ArchiveImage* archive = reinterpret_cast<const ArchiveImage*>(_archive);
	*this = *archive;
}

}