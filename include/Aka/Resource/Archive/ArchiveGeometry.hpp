#pragma once

#include <Aka/Resource/Archive/Archive.hpp>
#include <Aka/Graphic/Pipeline.h>

namespace aka {

struct Vertex {
	point3f position;
	norm3f normal;
	uv2f uv;
	color4f color;

	static gfx::VertexAttributeState getState();
};

struct ArchiveGeometry : Archive 
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,
		AddedNormalAndColor,

		Latest = AddedNormalAndColor
	};
	ArchiveGeometry();
	ArchiveGeometry(AssetID id);

	Vector<Vertex> vertices;
	Vector<uint32_t> indices;
	aabbox<> bounds; // local

protected:
	ArchiveLoadResult load_internal(ArchiveLoadContext& _context, BinaryArchive& path) override;
	ArchiveSaveResult save_internal(ArchiveSaveContext& _context, BinaryArchive& path) override;
	ArchiveLoadResult load_dependency(ArchiveLoadContext& _context) override;
	ArchiveSaveResult save_dependency(ArchiveSaveContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };

	void copyFrom(const Archive* _archive) override;
};

}