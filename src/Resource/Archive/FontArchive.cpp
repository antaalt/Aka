#include <Aka/Resource/Archive/FontArchive.h>


#include <Aka/Core/Application.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/OS/Archive.h>
#include <Aka/OS/Logger.h>
#include <Aka/Resource/Resource/Font.h>

namespace aka {

bool FontArchive::load(Stream& stream, BuildData* data)
{
	BinaryArchive archive(stream);

	// Read header
	char sign[4];
	archive.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 'f')
		return false; // Invalid file
	FontArchiveVersion version = archive.read<FontArchiveVersion>();
	if (version > FontArchiveVersion::Latest)
		return false; // Incompatible version

	// Read sprite
	FontBuildData* fontData = reinterpret_cast<FontBuildData*>(data);
	fontData->height = archive.read<uint32_t>();
	fontData->ttf = Blob(archive.read<uint32_t>());
	archive.read<uint8_t>(reinterpret_cast<uint8_t*>(fontData->ttf.data()), fontData->ttf.size());
	return true; // Always succeed
}

bool FontArchive::save(Stream& stream, const BuildData* data)
{
	BinaryArchive archive(stream);

	// Write header
	char signature[4] = { 'a', 'k', 'a', 'f' };
	archive.write<char>(signature, 4);
	archive.write<FontArchiveVersion>(FontArchiveVersion::Latest);

	// Write sprite
	const FontBuildData* fontData = reinterpret_cast<const FontBuildData*>(data);
	archive.write<uint32_t>(fontData->height);
	archive.write<uint32_t>((uint32_t)fontData->ttf.size());
	archive.write<uint8_t>(reinterpret_cast<const uint8_t*>(fontData->ttf.data()), fontData->ttf.size());
	return true; // Always succeed
}

};