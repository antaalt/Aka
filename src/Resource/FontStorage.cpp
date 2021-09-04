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
	FileStream stream(path, FileMode::Read);
	// Read header
	char sign[4];
	stream.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 'f')
		return false; // Invalid file
	uint16_t version = stream.read<uint16_t>();
	if (version != ((major << 8) | (minor)))
		return false; // Incompatible version
	// Read font
	return true;
}
bool FontStorage::save(const Path& path) const
{
	FileStream stream(path, FileMode::Write);
	// Write header
	char signature[4] = { 'a', 'k', 'a', 'f' };
	stream.write<char>(signature, 4);
	stream.write<uint16_t>((major << 8) | minor);
	// Write font
	return true;
}

std::shared_ptr<Font> FontStorage::to() const
{
	Font::Ptr mesh = Font::create("", 34);
	return mesh;
}
void FontStorage::from(const std::shared_ptr<Font>& mesh)
{
}

size_t FontStorage::size(const std::shared_ptr<Font>& mesh)
{
	return 0;
}

}; // namespace aka