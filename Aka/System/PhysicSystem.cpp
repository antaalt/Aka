#include "PhysicSystem.h"

#include "../Platform/Time.h"
#include "../Core/Geometry.h"
#include "../Component/Transform2D.h"
#include "../Component/Collider2D.h"
#include "../Core/World.h"

namespace aka {

PhysicSystem::PhysicSystem(World *world) :
	System(world)
{
}

void PhysicSystem::update(Time::Unit deltaTime)
{
	// TODO move these forces out of there
	const vec2f force = vec2f(0.f, -9.81f) + vec2f(0.f, 2.f); // gravity + air resistance
	float dt = deltaTime.seconds();
	m_world->each<RigidBody2D>([&](Entity* entity, RigidBody2D* rigid)  -> void {
		rigid->acceleration += (force / rigid->mass); // F=ma, acceleration is in m/s^2
		rigid->velocity += rigid->acceleration * dt; // m/s
		rigid->velocity.x = clamp(rigid->velocity.x, -RigidBody2D::maxVelocity.x, RigidBody2D::maxVelocity.x);
		rigid->velocity.y = clamp(rigid->velocity.y, -RigidBody2D::maxVelocity.y, RigidBody2D::maxVelocity.y);
	});
}

}