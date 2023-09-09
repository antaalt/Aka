#include <Aka/Resource/Archive/TextureArchive.h>

#include <Aka/Core/Application.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/OS/Archive.h>
#include <Aka/OS/Logger.h>
#include <Aka/Resource/Resource/Texture.h>

namespace aka {

bool TextureArchive::load(Stream& stream, BuildData* data)
{
	BinaryArchive archive(stream);

	// Read header
	char sign[4];
	archive.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 't')
		return false; // Invalid file
	TextureArchiveVersion version = archive.read<TextureArchiveVersion>();
	if (version > TextureArchiveVersion::Latest)
		return false; // Incompatible version
	// Read texture
	TextureBuildData* textureData = reinterpret_cast<TextureBuildData*>(data);
	textureData->channels = archive.read<uint8_t>();
	textureData->flags = archive.read<TextureBuildFlag>();
	textureData->height = archive.read<uint32_t>();
	textureData->width = archive.read<uint32_t>();

	textureData->layers = archive.read<uint32_t>();

	bool isHDR = has(textureData->flags, TextureBuildFlag::ColorSpaceHDR);
	gfx::TextureType type = gfx::TextureType::Texture2D;
	if (textureData->layers > 1)
	{
		if (has(textureData->flags, TextureBuildFlag::Cubemap))
			type = gfx::TextureType::TextureCubeMap;
		else
			type = gfx::TextureType::Texture2DArray;
	}
	switch (type)
	{
	case gfx::TextureType::Texture2D: {
		Vector<uint8_t> encodedBytes(archive.read<uint32_t>());
		archive.read<uint8_t>(encodedBytes.data(), encodedBytes.size());
		if (isHDR)
		{
			// TODO use stbi
			ImageHdr image = ImageDecoder::fromMemoryHdr(encodedBytes.data(), encodedBytes.size());
			textureData->bytes.append(reinterpret_cast<uint8_t*>(image.data()), reinterpret_cast<uint8_t*>(image.data()) + image.size() * sizeof(float));
		}
		else
		{
			Image image = ImageDecoder::fromMemory(encodedBytes.data(), encodedBytes.size());
			textureData->bytes.append(image.data(), image.data() + image.size());
		}
		break;
	}
	case gfx::TextureType::TextureCubeMap:
	case gfx::TextureType::Texture2DArray: {
		for (size_t i = 0; i < textureData->layers; i++)
		{
			Vector<uint8_t> encodedBytes(archive.read<uint32_t>());
			archive.read<uint8_t>(encodedBytes.data(), encodedBytes.size());
			if (isHDR)
			{
				// TODO use stbi
				ImageHdr image = ImageDecoder::fromMemoryHdr(encodedBytes.data(), encodedBytes.size());
				textureData->bytes.append(reinterpret_cast<uint8_t*>(image.data()), reinterpret_cast<uint8_t*>(image.data()) + image.size() * sizeof(float));
			}
			else
			{
				Image image = ImageDecoder::fromMemory(encodedBytes.data(), encodedBytes.size());
				textureData->bytes.append(image.data(), image.data() + image.size());
			}
		}
		break;
	}
	default:
		Logger::error("Texture type not supported");
		return false;
	}
	return true;
}

bool TextureArchive::save(Stream& stream, const BuildData* data)
{
	BinaryArchive archive(stream);

	// Write header
	char signature[4] = { 'a', 'k', 'a', 't' };
	archive.write<char>(signature, 4);
	archive.write<TextureArchiveVersion>(TextureArchiveVersion::Latest);

	// Write texture
	const TextureBuildData* textureData = reinterpret_cast<const TextureBuildData*>(data);
	bool isHDR = has(textureData->flags, TextureBuildFlag::ColorSpaceHDR);
	archive.write<uint8_t>(textureData->channels);
	archive.write<TextureBuildFlag>(textureData->flags);
	archive.write<uint32_t>(textureData->height);
	archive.write<uint32_t>(textureData->width);
	archive.write<uint32_t>(textureData->layers);

	gfx::TextureType type = gfx::TextureType::Texture2D;
	if (textureData->layers > 1)
	{
		if (has(textureData->flags, TextureBuildFlag::Cubemap))
			type = gfx::TextureType::TextureCubeMap;
		else
			type = gfx::TextureType::Texture2DArray;
	}
	switch (type)
	{
	/*case gfx::TextureType::Texture2D: {
		if (isHDR)
		{
			// encode to .hdr
			Image image(textureData->width, textureData->height, textureData->channels, reinterpret_cast<const float*>(textureData->bytes.data()));
			std::vector<uint8_t> data = image.encodeHDR();
			archive.write<uint32_t>((uint32_t)data.size());
			archive.write<uint8_t>(data.data(), data.size());
		}
		else
		{
			// encode to .png
			Image image(textureData->width, textureData->height, textureData->channels, textureData->bytes.data());
			std::vector<uint8_t> data = image.encodePNG();
			archive.write<uint32_t>((uint32_t)data.size());
			archive.write<uint8_t>(data.data(), data.size());
		}
		break;
	}
	case gfx::TextureType::TextureCubeMap:
	case gfx::TextureType::Texture2DArray:
			for (size_t i = 0; i < textureData->layers; i++)
			{
				if (isHDR)
				{
					// encode to .hdr
					Image image(textureData->width, textureData->height, textureData->channels, reinterpret_cast<const float*>(textureData->bytes.data()));
					std::vector<uint8_t> data = image.encodeHDR();
					archive.write<uint32_t>((uint32_t)data.size());
					archive.write<uint8_t>(data.data(), data.size());
				}
				else
				{
					// encode to png.
					Image image(textureData->width, textureData->height, textureData->channels, textureData->bytes.data());
					std::vector<uint8_t> data = image.encodePNG();
					archive.write<uint32_t>((uint32_t)data.size());
					archive.write<uint8_t>(data.data(), data.size());
				}
			}
			break;*/
	default:
		Logger::error("Texture type not supported");
		return false;
	}
	return true;
}

};