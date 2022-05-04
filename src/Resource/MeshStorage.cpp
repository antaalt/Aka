#include <Aka/Resource/MeshStorage.h>

#include <Aka/OS/Logger.h>
#include <Aka/OS/Archive.h>
#include <Aka/OS/Stream/FileStream.h>
#include <Aka/Core/Application.h>
#include <Aka/Resource/ResourceManager.h>
#include <Aka/Resource/BufferStorage.h>

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
	FileStream stream(path, FileMode::Read, FileType::Binary);
	BinaryArchive archive(stream);
	// Read header
	char sign[4];
	archive.read<char>(sign, 4);
	if (sign[0] != 'a' || sign[1] != 'k' || sign[2] != 'a' || sign[3] != 'm' )
		return false; // Invalid file
	uint16_t version = archive.read<uint16_t>();
	if (version != ((major << 8) | (minor)))
		return false; // Incompatible version
	// Read vertices
	vertices.resize(archive.read<uint32_t>());
	for (VertexBinding& vertex : vertices)
	{
		vertex.attribute.semantic = (gfx::VertexSemantic)archive.read<uint8_t>();
		vertex.attribute.format = (gfx::VertexFormat)archive.read<uint8_t>();
		vertex.attribute.type = (gfx::VertexType)archive.read<uint8_t>();
		vertex.vertexCount = archive.read<uint32_t>();
		vertex.vertexOffset = archive.read<uint32_t>();
		vertex.vertexBufferOffset = archive.read<uint32_t>();
		vertex.vertexBufferSize = archive.read<uint32_t>();
		vertex.vertexBufferStride = archive.read<uint32_t>();
		vertex.vertexBufferName.resize(archive.read<uint16_t>());
		archive.read<char>(vertex.vertexBufferName.cstr(), vertex.vertexBufferName.length() + 1);
	}
	// Read indices
	indexFormat = (gfx::IndexFormat)archive.read<uint8_t>();
	indexCount = archive.read<uint32_t>();
	indexBufferOffset = archive.read<uint32_t>();
	indexBufferName.resize(archive.read<uint16_t>());
	archive.read<char>(indexBufferName.cstr(), indexBufferName.length());
	return true;
}
bool MeshStorage::save(const Path& path) const
{
	FileStream stream(path, FileMode::Write, FileType::Binary);
	BinaryArchive archive(stream);
	// Write header
	char signature[4] = { 'a', 'k', 'a', 'm' };
	archive.write<char>(signature, 4);
	archive.write<uint16_t>((major << 8) | minor);
	// Write vertices
	archive.write<uint32_t>((uint32_t)vertices.size());
	for (const VertexBinding& vertex : vertices)
	{
		archive.write<uint8_t>((uint8_t)vertex.attribute.semantic);
		archive.write<uint8_t>((uint8_t)vertex.attribute.format);
		archive.write<uint8_t>((uint8_t)vertex.attribute.type);
		archive.write<uint32_t>(vertex.vertexCount);
		archive.write<uint32_t>(vertex.vertexOffset);
		archive.write<uint32_t>(vertex.vertexBufferOffset);
		archive.write<uint32_t>(vertex.vertexBufferSize);
		archive.write<uint32_t>(vertex.vertexBufferStride);
		archive.write<uint16_t>((uint16_t)vertex.vertexBufferName.length());
		archive.write<char>(vertex.vertexBufferName.cstr(), vertex.vertexBufferName.length() + 1);
	}
	// Write indices
	archive.write<uint8_t>((uint8_t)indexFormat);
	archive.write<uint32_t>(indexCount);
	archive.write<uint32_t>(indexBufferOffset);
	archive.write<uint16_t>((uint16_t)indexBufferName.length() + 1);
	archive.write<char>(indexBufferName.cstr(), indexBufferName.length() + 1);
	return true;
}

Mesh* MeshStorage::allocate() const
{
	Application* app = Application::app();
	ResourceManager* resources = app->resource();

	Mesh* mesh = Mesh::create();
	mesh->bindings.count = (uint32_t)vertices.size();
	for (size_t i = 0; i < vertices.size(); i++)
	{
		mesh->bindings.attributes[i] = vertices[i].attribute;
		mesh->bindings.offsets[i] = vertices[i].vertexOffset;
		
		//mesh->bindings.offsets[i] = vertices[i].vertexCount;

		mesh->vertices[i] = resources->get<Buffer>(vertices[i].vertexBufferName)->buffer;
		AKA_ASSERT(mesh->vertices[i] != nullptr, "No vertex buffer");
		//mesh->bindings.offsets[i] = vertices[i].vertexBufferOffset;
		//mesh->bindings.offsets[i] = vertices[i].vertexBufferSize;
		//mesh->bindings.offsets[i] = vertices[i].vertexBufferStride;
	}
	mesh->count = indexCount;
	mesh->format = indexFormat;

	mesh->indices = resources->get<Buffer>(indexBufferName)->buffer;
	AKA_ASSERT(mesh->indices != nullptr, "No index buffer");
	return mesh;
}

void MeshStorage::deallocate(Mesh* mesh) const
{
	delete mesh;
	//Mesh::destroy(mesh);
}

void MeshStorage::serialize(const Mesh& mesh)
{
	/*Application* app = Application::app();
	ResourceManager* resources = app->resource();

	vertices.resize(mesh->getVertexAttributeCount());
	for (uint32_t i = 0; i < mesh->getVertexAttributeCount(); i++)
	{
		Vertex& vert = vertices[i];
		const VertexBufferView& bufferView = mesh->getVertexBuffer(i);
		vert.attribute = mesh->getVertexAttribute(i);
		vert.vertexCount = mesh->getVertexCount(i);
		vert.vertexOffset = mesh->getVertexOffset(i);
		vert.vertexBufferName = resources->name<Buffer>(bufferView.buffer);
		vert.vertexBufferOffset = bufferView.offset;
		vert.vertexBufferSize = bufferView.size;
		vert.vertexBufferStride = bufferView.stride;
	}
	IndexBufferView bufferView = mesh->getIndexBuffer();
	indexBufferName = resources->name<Buffer>(bufferView.buffer);
	indexBufferOffset = bufferView.offset;
	indexCount = mesh->getIndexCount();
	indexFormat = mesh->getIndexFormat();*/
}

size_t MeshStorage::size(const Mesh& mesh)
{
	size_t size = mesh.indices->size;
	for (uint32_t i = 0; i < mesh.bindings.count; i++)
		size += mesh.vertices[i]->size;
	return size;
}

}; // namespace aka