#pragma once

#include <Aka/Resource/Archive/Archive.hpp>
#include <Aka/Graphic/Pipeline.h>

namespace aka {

struct ArchiveStaticVertex
{
	float position[3];
	float normal[3];
	float uv[2];
	float color[4];

	//static gfx::VertexBufferLayout getState();
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
	explicit ArchiveGeometry(AssetID id);

	Vector<ArchiveStaticVertex> vertices;
	Vector<uint32_t> indices;
	aabbox<> bounds; // local

protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

}