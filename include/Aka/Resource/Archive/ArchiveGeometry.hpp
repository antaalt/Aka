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
};

struct ArchiveSkeletalVertex
{
	float position[3];
	float normal[3];
	float uv[2];
	float color[4];
	uint32_t boneIndex[4];
	float weights[4];
};

struct ArchiveGeometry : Archive 
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,
		AddedNormalAndColor,
		AddedSkeletalSupport,

		Latest = AddedSkeletalSupport
	};
	ArchiveGeometry();
	explicit ArchiveGeometry(AssetID id);

	AssetID skeleton;
	aabbox<> bounds; // local
	// TODO should have some stream describing the vertices layout.
	Vector<ArchiveStaticVertex> staticVertices;
	Vector<ArchiveSkeletalVertex> skeletalVertices;
	Vector<uint32_t> indices;

protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

}