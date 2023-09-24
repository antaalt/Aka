#pragma once

#include <Aka/Scene/Component.hpp>
#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Resource/Resource/StaticMesh.hpp>
#include <Aka/Renderer/Instance.hpp>

namespace aka {

class StaticMeshComponent : public Component
{
public:
	StaticMeshComponent(AssetID assetID);
	~StaticMeshComponent();

	void onBecomeActive(AssetLibrary* library, Renderer* _renderer) override;
	void onBecomeInactive(AssetLibrary* library, Renderer* _renderer) override;

	ResourceHandle<StaticMesh> getMesh();
	void setInstanceTransform(const mat4f& transform) { m_instance->transform = transform; }
private:
	AssetID m_assetID;
	ResourceHandle<StaticMesh> m_meshHandle;
	Instance* m_instance;
};

};