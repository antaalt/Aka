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
	ArchiveScene(AssetID id);

	aabbox<> bounds;
	Vector<ArchiveSceneNode> nodes;

protected:
	ArchiveLoadResult load_internal(ArchiveLoadContext& _context, BinaryArchive& path) override;
	ArchiveSaveResult save_internal(ArchiveSaveContext& _context, BinaryArchive& path) override;
	ArchiveLoadResult load_dependency(ArchiveLoadContext& _context) override;
	ArchiveSaveResult save_dependency(ArchiveSaveContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };

	void copyFrom(const Archive* _archive) override;
};

};