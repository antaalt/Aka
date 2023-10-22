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

	void parse(BinaryArchive& archive) override;
};

class StaticMeshComponent : public Component
{
public:
	StaticMeshComponent(Node* node);
	~StaticMeshComponent();

	void onBecomeActive(AssetLibrary* library, Renderer* _renderer) override;
	void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) override;
	void onRenderUpdate(AssetLibrary* library, Renderer* _renderer) override;

	ResourceHandle<StaticMesh> getMesh() const;
	aabbox<> getWorldBounds() const;
public:
	void fromArchive(const ArchiveComponent& archive) override;
	void toArchive(ArchiveComponent& archive) override;
private:
	AssetID m_assetID;
	ResourceHandle<StaticMesh> m_meshHandle;
	InstanceHandle m_instance;
};

AKA_DECL_COMPONENT(StaticMeshComponent);

};