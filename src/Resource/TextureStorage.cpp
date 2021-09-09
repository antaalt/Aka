#include <Aka/Resource/TextureStorage.h>

#include <Aka/OS/Logger.h>
#include <Aka/OS/Stream/FileStream.h>
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
	FileStream stream(path, FileMode::Read);
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
	switch (type)
	{
	case TextureType::Texture2D: {
		std::vector<uint8_t> bytes(stream.read<uint32_t>());
		stream.read<uint8_t>(bytes.data(), bytes.size());
		images.push_back(Image::load(bytes));
		break;
	}
	case TextureType::TextureCubeMap: {
		std::vector<uint8_t> bytes;
		for (size_t i = 0; i < 6; i++)
		{
			bytes.resize(stream.read<uint32_t>());
			stream.read<uint8_t>(bytes.data(), bytes.size());
			images.push_back(Image::load(bytes));
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
	FileStream stream(path, FileMode::Write);
	// Write header
	char signature[4] = { 'a', 'k', 'a', 't' };
	stream.write<char>(signature, 4);
	stream.write<uint16_t>((major << 8) | minor);
	// Write texture
	stream.write<uint8_t>((uint8_t)type);
	stream.write<uint8_t>((uint8_t)format);
	stream.write<uint8_t>((uint8_t)flags);
	switch (type)
	{
	case TextureType::Texture2D: {
		// encode to png.
		std::vector<uint8_t> data = images[0].save();
		stream.write<uint32_t>((uint32_t)data.size());
		stream.write<uint8_t>(data.data(), data.size());
		break;
	}
	case TextureType::TextureCubeMap:
		for (size_t i = 0; i < 6; i++)
		{
			// encode to png.
			std::vector<uint8_t> data = images[i].save();
			stream.write<uint32_t>((uint32_t)data.size());
			stream.write<uint8_t>(data.data(), data.size());
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
		AKA_ASSERT(images.size() == 1, "");
		return Texture2D::create(images[0].width, images[0].height, format, flags, images[0].bytes.data());
	case TextureType::TextureCubeMap:
		AKA_ASSERT(images.size() == 6, "");
		return TextureCubeMap::create(
			images[0].width, images[0].height,
			format, flags,
			images[0].bytes.data(),
			images[1].bytes.data(),
			images[2].bytes.data(),
			images[3].bytes.data(),
			images[4].bytes.data(),
			images[5].bytes.data()
		);
	case TextureType::Texture2DMultisample:
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
		Image& img = images[0];
		img.width = texture->width();
		img.height = texture->height();
		img.bytes.resize(img.width * img.height * aka::size(format));
		reinterpret_cast<Texture2D*>(texture.get())->download(img.bytes.data());
		break;
	}
	case TextureType::TextureCubeMap: {
		images.resize(6);
		TextureCubeFace face{};
		for (size_t i = 0; i < 6; i++)
		{
			Image& img = images[i];
			img.width = texture->width();
			img.height = texture->height();
			img.bytes.resize(img.width * img.height * aka::size(format));
			reinterpret_cast<TextureCubeMap*>(texture.get())->download(face, img.bytes.data());
			images.push_back(img);
			face = (TextureCubeFace)((int)face + 1);
		}
		break;
	}
	case TextureType::Texture2DMultisample:
	default:
		break;
	}
	
}

size_t TextureStorage::size(const std::shared_ptr<Texture>& mesh)
{
	return mesh->width() * mesh->height() * aka::size(mesh->format());
}

}; // namespace aka
