#include "PhysicSystem.h"

#include "Time.h"
#include "Geometry.h"
#include "Transform2D.h"

#include "World.h"

namespace app {

PhysicSystem::PhysicSystem(World *world, Time::Unit timestep) :
	System(world),
	m_timestep(timestep)
{
}

void PhysicSystem::update()
{
	static Time::Unit lastTick = Time::now();
	const Time::Unit now = Time::now();
	// signature
	// rigidbody & transform & collider
	// OR
	// collider & transform
	const vec2f force = vec2f(0.f, -9.81f);
	float dt = m_timestep.seconds();
	// dt is the timestep
	while (lastTick + m_timestep < now)
	{
#if 1
		m_world->each<RigidBody2D>([&](Entity* entity, RigidBody2D* rigid)  -> void {
			rigid->acceleration += (force / rigid->mass) * dt;
			rigid->velocity += rigid->acceleration * dt;
			rigid->velocity.x = clamp(rigid->velocity.x, -RigidBody2D::maxVelocity.x, RigidBody2D::maxVelocity.x);
			rigid->velocity.y = clamp(rigid->velocity.y, -RigidBody2D::maxVelocity.y, RigidBody2D::maxVelocity.y);
		});
		m_world->each<Transform2D, RigidBody2D, Collider2D>([&](Entity* entity, Transform2D* transform, RigidBody2D* rigid, Collider2D *collider) {
			transform->position += rigid->velocity;// *dt;
			rigid->position += rigid->velocity;
			collider->position += rigid->velocity;
			m_world->each<Collider2D>([&](Entity* otherEntity, Collider2D* otherCollider) {
				// Skip self intersection
				if (otherEntity == entity)
					return;
				const RigidBody2D* otherRigid = otherEntity->get<RigidBody2D>();
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
						rigid->position += c.separation;
						collider->position += c.separation;
					}
					// Get penetration component
					vec2f p = normal * ps;
					// tangent component
					vec2f t = v - p;
					// Restitution
					float r = 1.f + max<float>(rigid->bouncing, otherRigid ? otherRigid->bouncing : 0.f); // max bouncing value of object a & b
					// Friction
					float f = min<float>(rigid->friction, otherRigid ? otherRigid->friction : 0.f); // max friction value of object a & b
					// Change the velocity of shape a
					rigid->acceleration = vec2f(0);
					rigid->velocity = rigid->velocity - p * r + t * f;
				}
			});
		});
		lastTick += m_timestep;

#else


		// Update velocity vectors
		for (Entity* entity : m_entities)
		{
			if (!entity->has<RigidBody2D>())
				continue;
			RigidBody2D* rigid = entity->get<RigidBody2D>();
			rigid->acceleration += (force / rigid->mass) * dt;
			rigid->velocity += rigid->acceleration * dt;
			rigid->velocity.x = clamp(rigid->velocity.x, -RigidBody2D::maxVelocity.x, RigidBody2D::maxVelocity.x);
			rigid->velocity.y = clamp(rigid->velocity.y, -RigidBody2D::maxVelocity.y, RigidBody2D::maxVelocity.y);
		}
		// Move dynamic objects
		for (Entity* entity : m_entities)
		{
			if (!entity->has<RigidBody2D>())
				continue;
			Transform2D* transform = entity->get<Transform2D>();
			RigidBody2D* rigid = entity->get<RigidBody2D>();
			Collider2D* collider = entity->get<Collider2D>();
			// Move rigid & its collider
			transform->position += rigid->velocity;
			rigid->position += rigid->velocity;// *dt;
			collider->position += rigid->velocity;// *dt;
			// Check collisions with static objects
			for (Entity* otherEntity : m_entities)
			{
				// Skip self intersection
				if (otherEntity == entity)
					continue;
				Collider2D* otherCollider = otherEntity->get<Collider2D>();
				const RigidBody2D* otherRigid = otherEntity->get<RigidBody2D>();
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
						rigid->position += c.separation;
						collider->position += c.separation;
					}
					// Get penetration component
					vec2f p = normal * ps;
					// tangent component
					vec2f t = v - p;
					// Restitution
					float r = 1.f + max<float>(rigid->bouncing, otherRigid ? otherRigid->bouncing : 0.f); // max bouncing value of object a & b
					// Friction
					float f = min<float>(rigid->friction, otherRigid ? otherRigid->friction : 0.f); // max friction value of object a & b
					// Change the velocity of shape a
					rigid->acceleration = vec2f(0);
					rigid->velocity = rigid->velocity - p * r + t * f;
				}
			}
		}
		lastTick += m_timestep;
#endif
	}
}

}