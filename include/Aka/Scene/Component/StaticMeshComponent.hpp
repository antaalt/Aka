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

	void setInstance(Instance* instance);
	void setMesh(ResourceHandle<StaticMesh> mesh);

	ResourceHandle<StaticMesh> getMesh();
private:
	AssetID m_assetID;
	ResourceHandle<StaticMesh> m_mesh;
	Instance* m_instance;
};

};