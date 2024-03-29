#include <Aka/Resource/TextureStorage.h>

#include <Aka/OS/Logger.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/OS/Stream/MemoryStream.h>
#include <Aka/Resource/ResourceManager.h>

namespace aka {

template struct Resource<Texture>;
template class ResourceAllocator<Texture>;

template <>
std::unique_ptr<IStorage<Texture>> IStorage<Texture>::create()
{
	return std::make_unique<TextureStorage>();
}

bool TextureStorage::load(const Path& path)
{
	FileStream stream(path, FileMode::Read, FileType::Binary);
	// Read header
	char sign[4];
	stream.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 't')
		return false; // Invalid file
	uint16_t version = stream.read<uint16_t>();
	if (version != ((major << 8) | (minor)))
		return false; // Incompatible version
	// Read texture
	type = (TextureType)stream.read<uint8_t>();
	format = (TextureFormat)stream.read<uint8_t>();
	flags = (TextureFlag)stream.read<uint8_t>();
	bool isHDR = stream.read<bool>();
	switch (type)
	{
	case TextureType::Texture2D: {
		std::vector<uint8_t> bytes(stream.read<uint32_t>());
		stream.read<uint8_t>(bytes.data(), bytes.size());
		if (isHDR)
			images.push_back(Image::loadHDR(bytes.data(), bytes.size()));
		else
			images.push_back(Image::load(bytes.data(), bytes.size()));
		break;
	}
	case TextureType::TextureCubeMap: {
		std::vector<uint8_t> bytes;
		for (size_t i = 0; i < 6; i++)
		{
			std::vector<uint8_t> bytes(stream.read<uint32_t>());
			stream.read<uint8_t>(bytes.data(), bytes.size());
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
	// Write header
	char signature[4] = { 'a', 'k', 'a', 't' };
	bool isHDR = images[0].format() == ImageFormat::Float;
	stream.write<char>(signature, 4);
	stream.write<uint16_t>((major << 8) | minor);
	// Write texture
	stream.write<uint8_t>((uint8_t)type);
	stream.write<uint8_t>((uint8_t)format);
	stream.write<uint8_t>((uint8_t)flags);
	stream.write<bool>(isHDR);
	switch (type)
	{
	case TextureType::Texture2D: {
		if (isHDR)
		{
			// encode to .hdr
			std::vector<uint8_t> data = images[0].encodeHDR();
			stream.write<uint32_t>((uint32_t)data.size());
			stream.write<uint8_t>(data.data(), data.size());
		}
		else
		{
			// encode to .png
			std::vector<uint8_t> data = images[0].encodePNG();
			stream.write<uint32_t>((uint32_t)data.size());
			stream.write<uint8_t>(data.data(), data.size());
		}
		break;
	}
	case TextureType::TextureCubeMap:
		for (size_t i = 0; i < 6; i++)
		{
			if (isHDR)
			{
				// encode to .hdr
				std::vector<uint8_t> data = images[i].encodeHDR();
				stream.write<uint32_t>((uint32_t)data.size());
				stream.write<uint8_t>(data.data(), data.size());
			}
			else
			{
				// encode to png.
				std::vector<uint8_t> data = images[i].encodePNG();
				stream.write<uint32_t>((uint32_t)data.size());
				stream.write<uint8_t>(data.data(), data.size());
			}
		}
		break;
	default:
		Logger::error("Texture type not supported");
		break;
	}
	return true;
}

std::shared_ptr<Texture> TextureStorage::to() const
{
	switch (type)
	{
	case TextureType::Texture2D:
		if (images.size() != 1)
			return nullptr;
		return Texture2D::create(images[0].width(), images[0].height(), format, flags, images[0].data());
	case TextureType::TextureCubeMap:
		if (images.size() != 6)
			return nullptr;
		return TextureCubeMap::create(
			images[0].width(), images[0].height(),
			format, flags,
			images[0].data(),
			images[1].data(),
			images[2].data(),
			images[3].data(),
			images[4].data(),
			images[5].data()
		);
	default:
		return nullptr;
	}
}
void TextureStorage::from(const std::shared_ptr<Texture>& texture)
{
	type = texture->type();
	format = texture->format();
	flags = texture->flags();
	switch (type)
	{
	case TextureType::Texture2D: {
		images.resize(1);
		ImageFormat format = ImageFormat::None;
		switch (texture->format())
		{
		case TextureFormat::RGBA32F:
			format = ImageFormat::Float;
			break;
		case TextureFormat::RGBA8:
		case TextureFormat::RGBA8U:
			format = ImageFormat::UnsignedByte;
			break;
		default:
			// TODO add conversion to support more formats
			Logger::error("Texture format not supported.");
			return;
		}
		images[0] = Image(texture->width(), texture->height(), 4, format);
		reinterpret_cast<Texture2D*>(texture.get())->download(images[0].data());
		break;
	}
	case TextureType::TextureCubeMap: {
		images.resize(6);
		ImageFormat format = ImageFormat::None;
		switch (texture->format())
		{
		case TextureFormat::RGBA32F:
			format = ImageFormat::Float;
			break;
		case TextureFormat::RGBA8:
		case TextureFormat::RGBA8U:
			format = ImageFormat::UnsignedByte;
			break;
		default:
			// TODO add conversion to support more formats
			Logger::error("Texture format not supported.");
			return;
		}
		for (size_t i = 0; i < 6; i++)
		{
			images[i] = Image(texture->width(), texture->height(), 4, format);
			reinterpret_cast<TextureCubeMap*>(texture.get())->download(images[i].data(), (uint32_t)i);
		}
		break;
	}
	default:
		Logger::error("Texture type not supported");
		break;
	}
	
}

size_t TextureStorage::size(const std::shared_ptr<Texture>& mesh)
{
	return mesh->width() * mesh->height() * aka::size(mesh->format());
}

}; // namespace aka
