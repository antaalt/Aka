#pragma once

#include <Aka/Scene/ECS/World.hpp>
#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Resource/Resource/StaticMesh.hpp>
#include <Aka/Renderer/Instance.hpp>
#include <Aka/Resource/Archive/ArchiveScene.hpp>

#include <entt.hpp>
#include <functional>

namespace aka {

struct StaticMeshComponent
{
	AssetID assetID = AssetID::Invalid;
	ResourceHandle<StaticMesh> meshHandle;
	InstanceHandle instance = InstanceHandle::Invalid;
};
AKA_DECL_COMPONENT(StaticMeshComponent)

template <>
struct ecs::ArchiveComponent<StaticMeshComponent>
{
	AssetID assetID;

	void from(const StaticMeshComponent& component)
	{
		assetID = component.assetID;
	}
	void to(StaticMeshComponent& component) const
	{
		component.assetID = assetID;
	}
	void parse(BinaryArchive& archive)
	{
		archive.parse(assetID);
	}
};

};