#include <Aka/Resource/FontStorage.h>

#include <Aka/OS/Logger.h>
#include <Aka/OS/Archive.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/Resource/ResourceManager.h>

namespace aka {

template struct Resource<Font>;
template class ResourceAllocator<Font>;

template <>
std::unique_ptr<IStorage<Font>> IStorage<Font>::create()
{
	return std::make_unique<FontStorage>();
}

bool FontStorage::load(const Path& path)
{
	FileStream stream(path, FileMode::Read, FileType::Binary);
	BinaryArchive archive(stream);
	// Read header
	char sign[4];
	archive.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 'f')
		return false; // Invalid file
	uint16_t version = archive.read<uint16_t>();
	if (version != ((major << 8) | (minor)))
		return false; // Incompatible version
	// Read font
	uint32_t size = archive.read<uint32_t>();
	ttf.resize(size);
	archive.read<byte_t>(ttf.data(), size);
	return true;
}
bool FontStorage::save(const Path& path) const
{
	FileStream stream(path, FileMode::Write, FileType::Binary);
	BinaryArchive archive(stream);
	// Write header
	char signature[4] = { 'a', 'k', 'a', 'f' };
	archive.write<char>(signature, 4);
	archive.write<uint16_t>((major << 8) | minor);
	// Write font
	archive.write<uint32_t>((uint32_t)ttf.size());
	archive.write<byte_t>(ttf.data(), ttf.size());
	return true;
}

Font* FontStorage::allocate() const
{
	Font* font = Font::create(ttf.data(), ttf.size(), 34);
	return font;
}
void FontStorage::deallocate(Font* font) const
{
	throw std::runtime_error("Not supported");
}
void FontStorage::serialize(const Font* font)
{
	throw std::runtime_error("Not supported");
}

size_t FontStorage::size(const Font* font)
{
	return font->atlas()->width * font->atlas()->height * gfx::Texture::size(font->atlas()->format);
}

}; // namespace aka