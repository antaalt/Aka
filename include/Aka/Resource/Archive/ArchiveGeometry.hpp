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

struct ArchiveSkeletalBone
{
	mat4f offset;
};

enum class ArchiveGeometryFlags
{
	None		= 0,

	IsSkeletal	= 1 << 0,

	All			= IsSkeletal,
};
AKA_IMPLEMENT_BITMASK_OPERATOR(ArchiveGeometryFlags);

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

	ArchiveGeometryFlags flags;
	aabbox<> bounds; // local
	// TODO should have some stream describing the vertices layout.
	Vector<ArchiveStaticVertex> staticVertices;
	Vector<ArchiveSkeletalVertex> skeletalVertices;
	Vector<ArchiveSkeletalBone> skeletalBones;
	Vector<uint32_t> indices;

protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

}