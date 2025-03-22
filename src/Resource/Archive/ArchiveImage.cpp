#include <Aka/Resource/Archive/ArchiveImage.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

ArchiveImage::ArchiveImage() :
	ArchiveImage(AssetID::Invalid)
{
}
ArchiveImage::ArchiveImage(AssetID id) : 
	Archive(AssetType::Image, id),
	data{},
	width(0),
	height(0),
	channels(0),
	format(ArchiveImageFormat::Uncompressed)
{
}
// This hugely improve loading time, but we lose in memory.
// Should store in compressed texture format instead (KTX & DDS)
// Could also have a flag that is telling if the image is compressed or not depending on the size on disk ?
ArchiveParseResult ArchiveImage::parse(BinaryArchive& _archive)
{
	_archive.parse<ArchiveImageFormat>(this->format);
	if (this->format == ArchiveImageFormat::Compressed)
	{
		if (_archive.isReading())
		{
			Vector<uint8_t> compressedData;
			_archive.parse(compressedData);
			Image img = ImageDecoder::fromMemory(compressedData.data(), compressedData.size()).getData();
			this->data = std::move(img.bytes);
			this->width = img.width;
			this->height = img.height;
			this->channels = img.getComponents();
		}
		else // isWriting
		{
			Image img;
			img.bytes = this->data;
			img.width = this->width;
			img.height = this->height;
			img.components = ImageComponent::RGBA;
			AKA_ASSERT(this->channels == 4, "");
			Blob blob = ImageEncoder::toMemory(img, ImageFileFormat::Png, ImageQuality::High).getData();
			_archive.parseBlob(blob.data(), blob.size());
		}
	}
	else
	{
		_archive.parse<uint32_t>(this->width);
		_archive.parse<uint32_t>(this->height);
		_archive.parse<uint32_t>(this->channels);
		_archive.parse(this->data);
	}
	return ArchiveParseResult::Success;
}

ArchiveParseResult ArchiveImage::load_dependency(ArchiveLoadContext& _context)
{
	return ArchiveParseResult::Success;
}

}