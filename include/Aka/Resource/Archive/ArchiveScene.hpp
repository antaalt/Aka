#pragma once 
	
#include <Aka/Resource/Archive/ArchiveStaticMesh.hpp>
#include <Aka/Scene/Component.hpp>

namespace aka {

enum class ArchiveSceneID : uint32_t { Invalid = (uint32_t)-1 };


struct ArchiveSceneComponent
{
	ComponentID id;
	Vector<byte_t> archive; // archive data as blob
};

struct ArchiveSceneNode 
{
	String name;
	mat4f transform;
	ArchiveSceneID parentID;
	Vector<ArchiveSceneComponent> components;
};

struct ArchiveScene : Archive 
{
	enum class Version : ArchiveVersionType
	{
		ArchiveCreation = 0,

		Latest = ArchiveCreation
	};
	ArchiveScene();
	explicit ArchiveScene(AssetID id);

	aabbox<> bounds;
	Vector<ArchiveSceneNode> nodes;

protected:
	ArchiveParseResult parse(BinaryArchive& path) override;
	ArchiveParseResult load_dependency(ArchiveLoadContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };
};

};