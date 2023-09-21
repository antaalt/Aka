#include <Aka/Resource/Archive/ArchiveGeometry.hpp>

#include <Aka/OS/Archive.h>
#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

gfx::VertexAttributeState Vertex::getState()
{
	gfx::VertexAttributeState attributes{};
	attributes.add(gfx::VertexSemantic::Position, gfx::VertexFormat::Float, gfx::VertexType::Vec3);
	attributes.add(gfx::VertexSemantic::Normal, gfx::VertexFormat::Float, gfx::VertexType::Vec3);
	attributes.add(gfx::VertexSemantic::TexCoord0, gfx::VertexFormat::Float, gfx::VertexType::Vec2);
	attributes.add(gfx::VertexSemantic::Color0, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	return attributes;
}

ArchiveGeometry::ArchiveGeometry() : 
	ArchiveGeometry(AssetID::Invalid)
{
}
ArchiveGeometry::ArchiveGeometry(AssetID id) :
	Archive(AssetType::Geometry, id, getLatestVersion()),
	vertices{},
	indices{},
	bounds()
{
}
ArchiveLoadResult ArchiveGeometry::load_internal(ArchiveLoadContext& _context, BinaryArchive& _archive)
{
	// Bounds
	_archive.read(this->bounds.min);
	_archive.read(this->bounds.max);

	// Indices
	uint32_t indexCount = _archive.read<uint32_t>();
	this->indices.resize(indexCount);
	_archive.read<uint32_t>(this->indices.data(), indexCount);

	// Vertices
	uint32_t vertexCount = _archive.read<uint32_t>();
	this->vertices.resize(vertexCount);
	_archive.read<Vertex>(this->vertices.data(), vertexCount);

	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveGeometry::save_internal(ArchiveSaveContext& _context, BinaryArchive& _archive)
{
	// Bounds
	_archive.write(this->bounds.min);
	_archive.write(this->bounds.max);

	_archive.write<uint32_t>((uint32_t)this->indices.size());
	_archive.write<uint32_t>(this->indices.data(), this->indices.size());

	_archive.write<uint32_t>((uint32_t)this->vertices.size());
	_archive.write<Vertex>(this->vertices.data(), this->vertices.size());

	return ArchiveSaveResult::Success;
}

ArchiveLoadResult ArchiveGeometry::load_dependency(ArchiveLoadContext& _context)
{
	return ArchiveLoadResult::Success;
}

ArchiveSaveResult ArchiveGeometry::save_dependency(ArchiveSaveContext& _context)
{
	return ArchiveSaveResult::Success;
}

void ArchiveGeometry::copyFrom(const Archive* _archive)
{
	AKA_ASSERT(_archive->id() == id(), "Invalid id");
	AKA_ASSERT(_archive->type() == type(), "Invalid type");
	AKA_ASSERT(_archive->version() == version(), "Invalid version");

	const ArchiveGeometry* geometry = reinterpret_cast<const ArchiveGeometry*>(_archive);
	*this = *geometry;
}

}