#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Scene/ECS/ecs.hpp>

namespace aka {

enum class ComponentUpdate
{
	Transform,
	Hierarchy,
};

struct TagComponent
{
	String name;
	ComponentUpdate update;
};
AKA_DECL_COMPONENT(TagComponent)

template <>
struct ecs::ArchiveComponent<TagComponent>
{
	String name;

	void from(const TagComponent& component)
	{
		name = component.name;
	}
	void to(TagComponent& component) const
	{
		component.name = name;
	}
	void parse(BinaryArchive& archive)
	{
		archive.parse(name);
	}
};

};