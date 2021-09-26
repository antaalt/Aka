#include <Aka/Resource/FontStorage.h>

#include <Aka/OS/Logger.h>
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
	// Read header
	char sign[4];
	stream.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 'f')
		return false; // Invalid file
	uint16_t version = stream.read<uint16_t>();
	if (version != ((major << 8) | (minor)))
		return false; // Incompatible version
	// Read font
	uint32_t size = stream.read<uint32_t>();
	ttf.resize(size);
	stream.read<byte_t>(ttf.data(), size);
	return true;
}
bool FontStorage::save(const Path& path) const
{
	FileStream stream(path, FileMode::Write, FileType::Binary);
	// Write header
	char signature[4] = { 'a', 'k', 'a', 'f' };
	stream.write<char>(signature, 4);
	stream.write<uint16_t>((major << 8) | minor);
	// Write font
	stream.write<uint32_t>((uint32_t)ttf.size());
	stream.write<byte_t>(ttf.data(), ttf.size());
	return true;
}

std::shared_ptr<Font> FontStorage::to() const
{
	Font::Ptr mesh = Font::create(ttf.data(), ttf.size(), 34);
	return mesh;
}
void FontStorage::from(const std::shared_ptr<Font>& font)
{
	throw std::runtime_error("Not supported");
}

size_t FontStorage::size(const std::shared_ptr<Font>& font)
{
	return font->atlas()->width() * font->atlas()->height() * aka::size(font->atlas()->format());
}

}; // namespace aka