#include <Aka/Resource/TextureStorage.h>

#include <Aka/Core/Application.h>
#include <Aka/OS/Logger.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/OS/Archive.h>
#include <Aka/OS/Stream/MemoryStream.h>
#include <Aka/Resource/ResourceManager.h>

namespace aka {

template struct Resource<gfx::Texture>;
template class ResourceAllocator<gfx::Texture>;

template <>
std::unique_ptr<IStorage<gfx::Texture>> IStorage<gfx::Texture>::create()
{
	return std::make_unique<TextureStorage>();
}

bool TextureStorage::load(const Path& path)
{
	FileStream stream(path, FileMode::Read, FileType::Binary);
	BinaryArchive archive(stream, Endian::native());

	// Read header
	char sign[4];
	archive.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 't')
		return false; // Invalid file
	uint16_t version = archive.read<uint16_t>();
	if (version != ((major << 8) | (minor)))
		return false; // Incompatible version
	// Read texture
	type = (gfx::TextureType)archive.read<uint8_t>();
	format = (gfx::TextureFormat)archive.read<uint8_t>();
	flags = (gfx::TextureFlag)archive.read<uint8_t>();
	levels = archive.read<uint8_t>();

	bool isHDR = archive.read<bool>();
	switch (type)
	{
	case gfx::TextureType::Texture2D: {
		std::vector<uint8_t> bytes(archive.read<uint32_t>());
		archive.read<uint8_t>(bytes.data(), bytes.size());
		if (isHDR)
			images.push_back(Image::loadHDR(bytes.data(), bytes.size()));
		else
			images.push_back(Image::load(bytes.data(), bytes.size()));
		break;
	}
	case gfx::TextureType::TextureCubeMap: {
		std::vector<uint8_t> bytes;
		for (size_t i = 0; i < 6; i++)
		{
			std::vector<uint8_t> bytes(archive.read<uint32_t>());
			archive.read<uint8_t>(bytes.data(), bytes.size());
			if (isHDR)
				images.push_back(Image::loadHDR(bytes.data(), bytes.size()));
			else
				images.push_back(Image::load(bytes.data(), bytes.size()));
		}
		break;
	}
	default:
		Logger::error("Texture type not supported");
		break;
	}
	return true;
}
bool TextureStorage::save(const Path& path) const
{
	FileStream stream(path, FileMode::Write, FileType::Binary);
	BinaryArchive archive(stream, Endian::native());

	// Write header
	char signature[4] = { 'a', 'k', 'a', 't' };
	bool isHDR = images[0].format() == ImageFormat::Float;
	archive.write<char>(signature, 4);
	archive.write<uint16_t>((major << 8) | minor);
	// Write texture
	archive.write<uint8_t>((uint8_t)type);
	archive.write<uint8_t>((uint8_t)format);
	archive.write<uint8_t>((uint8_t)flags);
	archive.write<uint8_t>((uint8_t)levels);
	archive.write<bool>(isHDR);
	switch (type)
	{
	case gfx::TextureType::Texture2D: {
		if (isHDR)
		{
			// encode to .hdr
			std::vector<uint8_t> data = images[0].encodeHDR();
			archive.write<uint32_t>((uint32_t)data.size());
			archive.write<uint8_t>(data.data(), data.size());
		}
		else
		{
			// encode to .png
			std::vector<uint8_t> data = images[0].encodePNG();
			archive.write<uint32_t>((uint32_t)data.size());
			archive.write<uint8_t>(data.data(), data.size());
		}
		break;
	}
	case gfx::TextureType::TextureCubeMap:
		for (size_t i = 0; i < 6; i++)
		{
			if (isHDR)
			{
				// encode to .hdr
				std::vector<uint8_t> data = images[i].encodeHDR();
				archive.write<uint32_t>((uint32_t)data.size());
				archive.write<uint8_t>(data.data(), data.size());
			}
			else
			{
				// encode to png.
				std::vector<uint8_t> data = images[i].encodePNG();
				archive.write<uint32_t>((uint32_t)data.size());
				archive.write<uint8_t>(data.data(), data.size());
			}
		}
		break;
	default:
		Logger::error("Texture type not supported");
		break;
	}
	return true;
}

gfx::Texture* TextureStorage::allocate() const
{
	gfx::GraphicDevice* device = Application::app()->graphic();
	switch (type)
	{
	case gfx::TextureType::Texture2D: {
		if (images.size() != 1)
			return nullptr;
		const void* data = images[0].data();
		return device->createTexture(
			images[0].width(),
			images[0].height(),
			1,
			gfx::TextureType::Texture2D,
			levels,
			1,
			format,
			flags,
			&data
		);
	}
	case gfx::TextureType::TextureCubeMap: {
		if (images.size() != 6)
			return nullptr;
		const void* data[6] = {
			images[0].data(),
			images[1].data(),
			images[2].data(),
			images[3].data(),
			images[4].data(),
			images[5].data()
		};
		return device->createTexture(
			images[0].width(), images[0].height(), 1,
			gfx::TextureType::TextureCubeMap,
			levels,
			6,
			format, flags,
			data
		);
	}
	default:
		return nullptr;
	}
}

void TextureStorage::deallocate(gfx::Texture* texture) const
{
	gfx::Texture::destroy(texture);
}
void TextureStorage::serialize(const gfx::Texture* texture)
{
	gfx::GraphicDevice* device = Application::app()->graphic();
	switch (texture->type)
	{
	case gfx::TextureType::Texture2D: {
		images.resize(1);
		ImageFormat format = ImageFormat::None;
		switch (texture->format)
		{
		case gfx::TextureFormat::RGBA32F:
			format = ImageFormat::Float;
			break;
		case gfx::TextureFormat::RGBA8:
		case gfx::TextureFormat::RGBA8U:
			format = ImageFormat::UnsignedByte;
			break;
		default:
			// TODO add conversion to support more formats
			Logger::error("Texture format not supported.");
			return;
		}
		images[0] = Image(texture->width, texture->height, 4, format);
		device->download(texture, images[0].data(), 0, 0, texture->width, texture->height);
		break;
	}
	case gfx::TextureType::TextureCubeMap: {
		images.resize(6);
		ImageFormat format = ImageFormat::None;
		switch (texture->format)
		{
		case gfx::TextureFormat::RGBA32F:
			format = ImageFormat::Float;
			break;
		case gfx::TextureFormat::RGBA8:
		case gfx::TextureFormat::RGBA8U:
			format = ImageFormat::UnsignedByte;
			break;
		default:
			// TODO add conversion to support more formats
			Logger::error("Texture format not supported.");
			return;
		}
		for (uint32_t i = 0; i < 6; i++)
		{
			images[i] = Image(texture->width, texture->height, 4, format);
			device->download(texture, images[i].data(), 0, 0, texture->width, texture->height, 0, i);
		}
		break;
	}
	default:
		Logger::error("Texture type not supported");
		break;
	}
	
}

size_t TextureStorage::size(const gfx::Texture* texture)
{
	return texture->width * texture->height * gfx::Texture::size(texture->format);
}

}; // namespace aka
