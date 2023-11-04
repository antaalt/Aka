#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Scene/ECS/ecs.hpp>

namespace aka {

struct HierarchyComponent
{
	mat4f inverseTransform = mat4f::identity();
	ecs::EntityID parent = ecs::EntityID::Invalid;
};
AKA_DECL_COMPONENT(HierarchyComponent)

template <>
struct ecs::ArchiveComponent<HierarchyComponent>
{
	mat4f inverse;
	ArchiveSceneEntityID entity;

	void from(const HierarchyComponent& component)
	{
		inverse = component.inverseTransform;
		entity = (ArchiveSceneEntityID)component.parent;
	}
	void to(HierarchyComponent& component) const
	{
		component.inverseTransform = inverse;
		component.parent = (ecs::EntityID)entity;
	}
	void parse(BinaryArchive& archive)
	{
		archive.parse(inverse);
		archive.parse(entity);
	}
};

};