#pragma once

#include <Aka/Core/Container/String.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Resource/Resource/Resource.hpp>
#include <Aka/Scene/ECS/ecs.hpp>

namespace aka {


struct Collision
{
	ecs::EntityID src = ecs::EntityID::Invalid;
	ecs::EntityID dst = ecs::EntityID::Invalid;
};

struct RigidBodyComponent
{
	float mass; // in kg
	vec3f velocity; // F = ma

	Vector<Collision> collisions;
};

enum class ColliderType
{
	Static,
	Dynamic,	// if rigid body attached
	Event,		// no response expected, just here as event triggering
};

struct ColliderComponent
{
	ColliderType type;
	Vector<Collision> collisions;
};

};