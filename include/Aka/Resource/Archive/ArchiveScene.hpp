#pragma once 
	
#include <Aka/Resource/Archive/ArchiveStaticMesh.hpp>

namespace aka {

struct ArchiveSceneTransform {
	mat4f matrix;
};

enum class ArchiveSceneID : uint32_t { Invalid = (uint32_t)-1 };
inline std::underlying_type<ArchiveSceneID>::type toIntegral(ArchiveSceneID value) { return static_cast<std::underlying_type<ArchiveSceneID>::type>(value); }
//ArchiveSceneID operator+(ArchiveSceneID lhs, ArchiveSceneID rhs) { return static_cast<ArchiveSceneID>(toIntegral(lhs) + toIntegral(rhs)); }
//ArchiveSceneID operator-(ArchiveSceneID lhs, ArchiveSceneID rhs) { return static_cast<ArchiveSceneID>(toIntegral(lhs) - toIntegral(rhs)); }

// TODO move to scene
// Should not be component dependent.
enum class SceneComponent {
	Unknown,

	Transform,
	Hierarchy,
	StaticMesh,
	PointLight,
	SunLight,

	First = Transform,
	Last = SunLight,
};
enum class SceneComponentMask {
	None  = 0,
	Transform  = 1 << EnumToIndex(SceneComponent::Transform),
	Hierarchy  = 1 << EnumToIndex(SceneComponent::Hierarchy),
	StaticMesh = 1 << EnumToIndex(SceneComponent::StaticMesh),

	PointLight = 1 << EnumToIndex(SceneComponent::PointLight),
	SunLight   = 1 << EnumToIndex(SceneComponent::SunLight),
};
AKA_IMPLEMENT_BITMASK_OPERATOR(SceneComponentMask)



struct ArchiveSceneEntity {
	String name;
	SceneComponentMask components;
	ArchiveSceneID id[EnumCount<SceneComponent>()];
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
	Vector<ArchiveStaticMesh> meshes;
	Vector<ArchiveSceneTransform> transforms;
	Vector<ArchiveSceneEntity> entities;
	// TODO: add lights, envmap, cameras, gameplay struct

protected:
	ArchiveLoadResult load_internal(ArchiveLoadContext& _context, BinaryArchive& path) override;
	ArchiveSaveResult save_internal(ArchiveSaveContext& _context, BinaryArchive& path) override;
	ArchiveLoadResult load_dependency(ArchiveLoadContext& _context) override;
	ArchiveSaveResult save_dependency(ArchiveSaveContext& _context) override;

	ArchiveVersionType getLatestVersion() const override { return static_cast<ArchiveVersionType>(Version::Latest); };

	void copyFrom(const Archive* _archive) override;
};

};