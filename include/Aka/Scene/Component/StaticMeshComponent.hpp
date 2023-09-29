#pragma once

#include <Aka/Scene/Component.hpp>
#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Resource/Resource/StaticMesh.hpp>
#include <Aka/Renderer/Instance.hpp>
#include <Aka/Resource/Archive/ArchiveScene.hpp>

namespace aka {

struct ArchiveStaticMeshComponent : ArchiveComponent
{
	ArchiveStaticMeshComponent();

	AssetID assetID;

	void load_internal(BinaryArchive& archive) override;
	void save_internal(BinaryArchive& archive) override;
};

class StaticMeshComponent : public Component
{
public:
	StaticMeshComponent();
	~StaticMeshComponent();

	void onBecomeActive(AssetLibrary* library, Renderer* _renderer) override;
	void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) override;

	ResourceHandle<StaticMesh> getMesh();
	void setInstanceTransform(const mat4f& transform) { m_instance->transform = transform; }
public:
	void load(const ArchiveComponent& archive) override;
	void save(ArchiveComponent& archive) override;
private:
	AssetID m_assetID;
	ResourceHandle<StaticMesh> m_meshHandle;
	Instance* m_instance;
};

AKA_DECL_COMPONENT(StaticMeshComponent);

};