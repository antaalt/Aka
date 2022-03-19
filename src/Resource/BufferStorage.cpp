#include <Aka/Resource/BufferStorage.h>

#include <Aka/OS/Logger.h>
#include <Aka/OS/Archive.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/Resource/ResourceManager.h>
#include <Aka/Core/Application.h>

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
	FileStream stream(path, FileMode::Read, FileType::Binary);
	BinaryArchive archive(stream);
	// Read header
	char sign[4];
	archive.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 'b')
		return false; // Invalid file
	uint16_t version = archive.read<uint16_t>();
	if (version != ((major << 8) | (minor)))
		return false; // Incompatible version
	// Read buffer
	type = (BufferType)archive.read<uint8_t>();
	usage = (BufferUsage)archive.read<uint8_t>();
	access = (BufferCPUAccess)archive.read<uint8_t>();
	bytes.resize(archive.read<uint32_t>());
	archive.read<uint8_t>(bytes.data(), bytes.size());
	return true;
}
bool BufferStorage::save(const Path& path) const
{
	FileStream stream(path, FileMode::Write, FileType::Binary);
	BinaryArchive archive(stream);
	// Write header
	char signature[4] = { 'a', 'k', 'a', 'b' };
	archive.write<char>(signature, 4);
	archive.write<uint16_t>((major << 8) | minor);
	// Write buffer
	archive.write<uint8_t>((uint8_t)type);
	archive.write<uint8_t>((uint8_t)usage);
	archive.write<uint8_t>((uint8_t)access);
	archive.write<uint32_t>((uint32_t)bytes.size());
	archive.write<uint8_t>(bytes.data(), bytes.size());
	return true;
}

Buffer* BufferStorage::allocate() const
{
	return Application::app()->graphic()->createBuffer(type, (uint32_t)bytes.size(), usage, access, bytes.data());
}

void BufferStorage::deallocate(Buffer* buffer) const
{
	Application::app()->graphic()->destroy(buffer);
}

void BufferStorage::serialize(const Buffer* buffer)
{
	type = buffer->type;
	usage = buffer->usage;
	access = buffer->access;
	bytes.resize(buffer->size);
	Application::app()->graphic()->download(buffer, bytes.data(), 0, buffer->size);
}

size_t BufferStorage::size(const Buffer* buffer)
{
	return buffer->size;
}

}; // namespace aka