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

	m_world->each<RigidBody2D, Transform2D>([dt](Entity* entity, RigidBody2D* rigid, Transform2D* transform)  -> void {
		transform->position += rigid->velocity * dt * 16.f; // scale by 16 as 16 is ~ 1m in game unit
	});
	m_world->each<Transform2D, RigidBody2D, Collider2D>([&](Entity* entity, Transform2D* transform, RigidBody2D* rigid, Collider2D* collider) {
		collider->position = transform->position;
		collider->size = transform->size;
		m_world->each<Transform2D, Collider2D>([&](Entity* otherEntity, Transform2D* otherTransform, Collider2D* otherCollider) {
			// Skip self intersection
			if (otherEntity == entity)
				return;
			const RigidBody2D* otherRigid = otherEntity->get<RigidBody2D>();
			otherCollider->position = otherTransform->position;
			otherCollider->size = otherTransform->size;
			Collision2D c = collider->overlaps(*otherCollider);
			if (c.collided)
			{
				// Adjust velocity
				// Get normal 
				vec2f normal = vec2f::normalize(c.separation);
				// Get relative velocity
				vec2f v = rigid->velocity; // substract by second object velocity if it has one
				// Get penetration speed
				float ps = vec2f::dot(v, normal);
				// objects moving towards each other ?
				if (ps <= 0.f)
				{
					// Move the rigid & its collider to avoid overlapping.
					transform->position += c.separation;
				}
				// Get penetration component
				vec2f p = normal * ps;
				// tangent component
				vec2f t = v - p;
				// Restitution
				// TODO move these settings to collider.
				float r = 1.f + max<float>(rigid->bouncing, otherRigid ? otherRigid->bouncing : 0.f); // max bouncing value of object a & b
				// Friction
				float f = min<float>(rigid->friction, otherRigid ? otherRigid->friction : 0.f); // max friction value of object a & b
				// Change the velocity of shape a
				rigid->acceleration = vec2f(0);
				rigid->velocity = rigid->velocity - p * r + t * f;
			}
		});
	});
}

};