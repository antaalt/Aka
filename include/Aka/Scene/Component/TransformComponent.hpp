#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Scene/ECS/ecs.hpp>

namespace aka {

struct TransformComponent
{
	mat4f transform = mat4f::identity();
};
AKA_DECL_COMPONENT(TransformComponent)

template <>
struct ecs::ArchiveComponent<TransformComponent>
{
	mat4f transform;

	void from(const TransformComponent& component)
	{
		transform = component.transform;
	}
	void to(TransformComponent& component) const
	{
		component.transform = transform;
	}
	void parse(BinaryArchive& archive)
	{
		archive.parse(transform);
	}
};


};