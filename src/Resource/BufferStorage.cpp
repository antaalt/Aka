#include <Aka/Resource/BufferStorage.h>

#include <Aka/OS/Logger.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/Resource/ResourceManager.h>

namespace aka {

template struct Resource<Buffer>;
template class ResourceAllocator<Buffer>;

template <>
std::unique_ptr<IStorage<Buffer>> IStorage<Buffer>::create()
{
	return std::make_unique<BufferStorage>();
}

bool BufferStorage::load(const Path& path)
{
	FileStream stream(path, FileMode::Read);
	// Read header
	char sign[4];
	stream.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 'b')
		return false; // Invalid file
	uint16_t version = stream.read<uint16_t>();
	if (version != ((major << 8) | (minor)))
		return false; // Incompatible version
	// Read buffer
	type = (BufferType)stream.read<uint8_t>();
	usage = (BufferUsage)stream.read<uint8_t>();
	access = (BufferCPUAccess)stream.read<uint8_t>();
	bytes.resize(stream.read<uint32_t>());
	stream.read<uint8_t>(bytes.data(), bytes.size());
	return true;
}
bool BufferStorage::save(const Path& path) const
{
	FileStream stream(path, FileMode::Write);
	// Write header
	char signature[4] = { 'a', 'k', 'a', 'b' };
	stream.write<char>(signature, 4);
	stream.write<uint16_t>((major << 8) | minor);
	// Write buffer
	stream.write<uint8_t>((uint8_t)type);
	stream.write<uint8_t>((uint8_t)usage);
	stream.write<uint8_t>((uint8_t)access);
	stream.write<uint32_t>((uint32_t)bytes.size());
	stream.write<uint8_t>(bytes.data(), bytes.size());
	return true;
}

std::shared_ptr<Buffer> BufferStorage::to() const
{
	return Buffer::create(type, bytes.size(), usage, access, bytes.data());
}

void BufferStorage::from(const std::shared_ptr<Buffer>& buffer)
{
	type = buffer->type();
	usage = buffer->usage();
	access = buffer->access();
	bytes.resize(buffer->size());
	buffer->download(bytes.data());
}

size_t BufferStorage::size(const std::shared_ptr<Buffer>& buffer)
{
	return buffer->size();
}

}; // namespace aka