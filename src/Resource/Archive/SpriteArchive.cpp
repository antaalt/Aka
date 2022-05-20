#include <Aka/Resource/Archive/SpriteArchive.h>

#include <Aka/Core/Application.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/OS/Archive.h>
#include <Aka/OS/Logger.h>
#include <Aka/Resource/Resource/Sprite.h>

namespace aka {

bool SpriteArchive::load(Stream& stream, BuildData* data)
{
	BinaryArchive archive(stream);

	// Read header
	char sign[4];
	archive.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 's')
		return false; // Invalid file
	SpriteArchiveVersion version = archive.read<SpriteArchiveVersion>();
	if (version > SpriteArchiveVersion::Latest)
		return false; // Incompatible version

	// Read sprite
	SpriteBuildData* spriteData = reinterpret_cast<SpriteBuildData*>(data);
	spriteData->width = archive.read<uint32_t>();
	spriteData->height = archive.read<uint32_t>();
	spriteData->animations.resize(archive.read<uint32_t>());
	for (SpriteBuildData::Animation& animation : spriteData->animations)
	{
		animation.name.resize(archive.read<uint32_t>()); 
		animation.name.cstr()[animation.name.length()] = 0; // Add null terminator
		archive.read<char>(animation.name.cstr(), animation.name.length());
		animation.frames.resize(archive.read<uint32_t>());
		for (SpriteBuildData::Frame& frame : animation.frames)
		{
			uint32_t size = archive.read<uint32_t>();
			frame.bytes = Blob(size);
			archive.read<byte_t>(reinterpret_cast<byte_t*>(frame.bytes.data()), frame.bytes.size());
			frame.duration = Time::milliseconds(archive.read<uint64_t>());
		}
	}
	return true;
}

bool SpriteArchive::save(Stream& stream, const BuildData* data)
{
	BinaryArchive archive(stream);

	// Write header
	char signature[4] = { 'a', 'k', 'a', 's' };
	archive.write<char>(signature, 4);
	archive.write<SpriteArchiveVersion>(SpriteArchiveVersion::Latest);

	// Write sprite
	const SpriteBuildData* spriteData = reinterpret_cast<const SpriteBuildData*>(data);
	archive.write<uint32_t>(spriteData->width);
	archive.write<uint32_t>(spriteData->height);
	archive.write<uint32_t>(static_cast<uint32_t>(spriteData->animations.size()));
	for (const SpriteBuildData::Animation& animation : spriteData->animations)
	{
		archive.write<uint32_t>(static_cast<uint32_t>(animation.name.length()));
		archive.write<char>(animation.name.cstr(), animation.name.length());
		archive.write<uint32_t>(static_cast<uint32_t>(animation.frames.size()));
		for (const SpriteBuildData::Frame& frame : animation.frames)
		{
			archive.write<uint32_t>(static_cast<uint32_t>(frame.bytes.size()));
			archive.write<byte_t>(reinterpret_cast<const byte_t*>(frame.bytes.data()), frame.bytes.size());
			archive.write<uint64_t>(frame.duration.milliseconds());
		}
	}
	return true;
}

};
