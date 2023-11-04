#pragma once

#include <Aka/Core/Geometry.h>

#include <Aka/OS/Time.h>
#include <Aka/Scene/ECS/World.hpp>
#include <Aka/Scene/Component/CameraComponent.hpp>
#include <Aka/Resource/Archive/ArchiveScene.hpp>
#include <Aka/Renderer/View.hpp>

namespace aka {

// ArcballMouseControllerComponent
struct ArcballComponent
{
	bool isUpdatable;
	CameraArcball controller;

	void setBounds(const aabbox<>& bounds);
};

AKA_DECL_COMPONENT(ArcballComponent);

struct ecs::ArchiveComponent<ArcballComponent>
{
	void from(const ArcballComponent& component) {}
	void to(ArcballComponent& component) const {}
	void parse(BinaryArchive& archive) {}
};


};