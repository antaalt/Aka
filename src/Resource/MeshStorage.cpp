#include <Aka/Resource/MeshStorage.h>

#include <Aka/OS/Logger.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/Resource/ResourceManager.h>

namespace aka {

template struct Resource<Mesh>;
template class ResourceAllocator<Mesh>;

template <>
std::unique_ptr<IStorage<Mesh>> IStorage<Mesh>::create()
{
	return std::make_unique<MeshStorage>();
}

bool MeshStorage::load(const Path& path)
{
	FileStream stream(path, FileMode::Read);
	// Read header
	char sign[4];
	stream.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 'm' )
		return false; // Invalid file
	uint16_t version = stream.read<uint16_t>();
	if (version != ((major << 8) | (minor)))
		return false; // Incompatible version
	// Read vertices
	vertices.resize(stream.read<uint32_t>());
	for (Vertex& vertex : vertices)
	{
		vertex.attribute.semantic = (VertexSemantic)stream.read<uint8_t>();
		vertex.attribute.format = (VertexFormat)stream.read<uint8_t>();
		vertex.attribute.type = (VertexType)stream.read<uint8_t>();
		vertex.vertexCount = stream.read<uint32_t>();
		vertex.vertexOffset = stream.read<uint32_t>();
		vertex.vertexBufferOffset = stream.read<uint32_t>();
		vertex.vertexBufferSize = stream.read<uint32_t>();
		vertex.vertexBufferStride = stream.read<uint32_t>();
		vertex.vertexBufferName.resize(stream.read<uint16_t>());
		stream.read<char>(vertex.vertexBufferName.cstr(), vertex.vertexBufferName.length() + 1);
	}
	// Read indices
	indexFormat = (IndexFormat)stream.read<uint8_t>();
	indexCount = stream.read<uint32_t>();
	indexBufferOffset = stream.read<uint32_t>();
	indexBufferName.resize(stream.read<uint16_t>());
	stream.read<char>(indexBufferName.cstr(), indexBufferName.length());
	return true;
}
bool MeshStorage::save(const Path& path) const
{
	FileStream stream(path, FileMode::Write);
	// Write header
	char signature[4] = { 'a', 'k', 'a', 'm' };
	stream.write<char>(signature, 4);
	stream.write<uint16_t>((major << 8) | minor);
	// Write vertices
	stream.write<uint32_t>((uint32_t)vertices.size());
	for (const Vertex& vertex : vertices)
	{
		stream.write<uint8_t>((uint8_t)vertex.attribute.semantic);
		stream.write<uint8_t>((uint8_t)vertex.attribute.format);
		stream.write<uint8_t>((uint8_t)vertex.attribute.type);
		stream.write<uint32_t>(vertex.vertexCount);
		stream.write<uint32_t>(vertex.vertexOffset);
		stream.write<uint32_t>(vertex.vertexBufferOffset);
		stream.write<uint32_t>(vertex.vertexBufferSize);
		stream.write<uint32_t>(vertex.vertexBufferStride);
		stream.write<uint16_t>((uint16_t)vertex.vertexBufferName.length());
		stream.write<char>(vertex.vertexBufferName.cstr(), vertex.vertexBufferName.length() + 1);
	}
	// Write indices
	stream.write<uint8_t>((uint8_t)indexFormat);
	stream.write<uint32_t>(indexCount);
	stream.write<uint32_t>(indexBufferOffset);
	stream.write<uint16_t>((uint16_t)indexBufferName.length() + 1);
	stream.write<char>(indexBufferName.cstr(), indexBufferName.length() + 1);
	return true;
}

std::shared_ptr<Mesh> MeshStorage::to() const
{
	Mesh::Ptr mesh = Mesh::create();
	std::vector<VertexAccessor> vertexAccessor(vertices.size());
	for (size_t i = 0; i < vertices.size(); i++)
	{
		const Vertex& vert = vertices[i];
		VertexAccessor& accessor = vertexAccessor[i];
		accessor.attribute = vert.attribute;
		accessor.count = vert.vertexCount;
		accessor.offset = vert.vertexOffset;
		accessor.bufferView.buffer = ResourceManager::get<Buffer>(vert.vertexBufferName);
		AKA_ASSERT(accessor.bufferView.buffer != nullptr, "No vertex buffer");
		accessor.bufferView.offset = vert.vertexBufferOffset;
		accessor.bufferView.size = vert.vertexBufferSize;
		accessor.bufferView.stride = vert.vertexBufferStride;
	}
	IndexAccessor indexAccessor;
	indexAccessor.bufferView.buffer = ResourceManager::get<Buffer>(indexBufferName);
	AKA_ASSERT(indexAccessor.bufferView.buffer != nullptr, "No index buffer");
	indexAccessor.bufferView.offset = indexBufferOffset;
	indexAccessor.bufferView.size = indexCount * aka::size(indexFormat);
	indexAccessor.count = indexCount;
	indexAccessor.format = indexFormat;
	mesh->upload(vertexAccessor.data(), vertexAccessor.size(), indexAccessor);
	return mesh;
}
void MeshStorage::from(const std::shared_ptr<Mesh>& mesh)
{
	vertices.resize(mesh->getVertexAttributeCount());
	for (uint32_t i = 0; i < mesh->getVertexAttributeCount(); i++)
	{
		Vertex& vert = vertices[i];
		const VertexBufferView& bufferView = mesh->getVertexBuffer(i);
		vert.attribute = mesh->getVertexAttribute(i);
		vert.vertexCount = mesh->getVertexCount(i);
		vert.vertexOffset = mesh->getVertexOffset(i);
		vert.vertexBufferName = ResourceManager::name<Buffer>(bufferView.buffer);
		vert.vertexBufferOffset = bufferView.offset;
		vert.vertexBufferSize = bufferView.size;
		vert.vertexBufferStride = bufferView.stride;
	}
	IndexBufferView bufferView = mesh->getIndexBuffer();
	indexBufferName = ResourceManager::name<Buffer>(bufferView.buffer);
	indexBufferOffset = bufferView.offset;
	indexCount = mesh->getIndexCount();
	indexFormat = mesh->getIndexFormat();
}

size_t MeshStorage::size(const std::shared_ptr<Mesh>& mesh)
{
	size_t size = mesh->getIndexBuffer().size;
	for (uint32_t i = 0; i < mesh->getVertexAttributeCount(); i++)
		size += mesh->getVertexBuffer(i).size;
	return size;
}

}; // namespace aka