#include "CollisionSystem.h"

#include "../Core/World.h"
#include "../Component/Transform2D.h"

namespace aka {

CollisionSystem::CollisionSystem(World* world) :
	System(world)
{
}

void CollisionSystem::update(Time::Unit deltaTime)
{
	float dt = deltaTime.seconds();

	m_world->each<RigidBody2D, Transform2D>([dt](Entity* entity, RigidBody2D* rigid, Transform2D* transform) {
		transform->position += rigid->velocity * dt * 16.f; // scale by 16 as 16 is ~ 1m in game unit
	});
	m_world->each<RigidBody2D, Transform2D, Collider2D>([&](Entity* entity, RigidBody2D* rigid, Transform2D* transform, Collider2D* collider) {
		collider->position = transform->position;
		collider->size = transform->size;
		m_world->each<Collider2D, Transform2D>([&](Entity* otherEntity, Collider2D* otherCollider, Transform2D* otherTransform) {
			// Skip self intersection
			if (otherEntity == entity)
				return;
			otherCollider->position = otherTransform->position;
			otherCollider->size = otherTransform->size;
			Collision2D c = collider->overlaps(*otherCollider);
			if (c.collided)
			{
				CollisionEvent e;
				e.left = entity;
				e.right = otherEntity;
				e.separation = c.separation;
				m_world->emit<CollisionEvent>(e);
			}
		});
	});
}

};