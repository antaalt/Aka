#include <Aka/Resource/AudioStorage.h>

#include <Aka/OS/Logger.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/Resource/ResourceManager.h>

namespace aka {

template struct Resource<AudioStream>;
template class ResourceAllocator<AudioStream>;

template <>
std::unique_ptr<IStorage<AudioStream>> IStorage<AudioStream>::create()
{
	return std::make_unique<AudioStorage>();
}

bool AudioStorage::load(const Path& path)
{
	FileStream stream(path, FileMode::Read);
	// Read header
	char sign[4];
	stream.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 'a')
		return false; // Invalid file
	uint16_t version = stream.read<uint16_t>();
	if (version != ((major << 8) | (minor)))
		return false; // Incompatible version
	// Read audio
	return true;
}
bool AudioStorage::save(const Path& path) const
{
	FileStream stream(path, FileMode::Write);
	// Write header
	char signature[4] = { 'a', 'k', 'a', 'a' };
	stream.write<char>(signature, 4);
	stream.write<uint16_t>((major << 8) | minor);
	// Write audio
	return true;
}

std::shared_ptr<AudioStream> AudioStorage::to() const
{
	return nullptr;
}
void AudioStorage::from(const std::shared_ptr<AudioStream>& mesh)
{
}

size_t AudioStorage::size(const std::shared_ptr<AudioStream>& mesh)
{
	return 0;
}

}; // namespace aka