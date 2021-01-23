#include "CollisionSystem.h"

#include "../Core/ECS/World.h"
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
		transform->translate(rigid->velocity * dt * 16.f); // scale by 16 as 16 is ~ 1m in game unit
	});
	m_world->each<RigidBody2D, Transform2D, Collider2D>([&](Entity* entity, RigidBody2D* rigid, Transform2D* transform, Collider2D* collider) {
		vec2f position = transform->model.multiplyPoint(collider->position);
		vec2f size = transform->model.multiplyVector(collider->size);
		m_world->each<Collider2D, Transform2D>([&](Entity* otherEntity, Collider2D* otherCollider, Transform2D* otherTransform) {
			// Skip self intersection
			if (otherEntity == entity)
				return;
			vec2f otherPosition = otherTransform->model.multiplyPoint(otherCollider->position);
			vec2f otherSize = otherTransform->model.multiplyVector(otherCollider->size);
			Collision2D c = overlap(position, size, otherPosition, otherSize);
			if (c.collided)
			{
				m_world->emit<CollisionEvent>(CollisionEvent(entity, otherEntity, c.separation));
			}
		});
	});
}

void CollisionEvent::resolve() const
{
	RigidBody2D* rigid = left->get<RigidBody2D>();
	Transform2D* transform = left->get<Transform2D>();
	Collider2D*  collider = left->get<Collider2D>();
	RigidBody2D* otherRigid = right->get<RigidBody2D>();
	Collider2D*  otherCollider = right->get<Collider2D>();

	// Adjust velocity
	// Get normal 
	vec2f normal = vec2f::normalize(separation);
	// Get relative velocity
	vec2f v = rigid->velocity; // substract by second object velocity if it has one
	// Get penetration speed
	float ps = vec2f::dot(v, normal);
	// objects moving towards each other ?
	if (ps > 0.f)
		return;
	// Get penetration component
	vec2f p = normal * ps;
	// tangent component
	vec2f t = v - p;
	// Restitution
	float r = 1.f + max<float>(collider->bouncing, otherCollider->bouncing); // max bouncing value of object a & b
	// Friction
	float f = min<float>(collider->friction, otherCollider->friction); // max friction value of object a & b
	// Change the velocity of shape a
	rigid->acceleration = vec2f(0);
	rigid->velocity = rigid->velocity - p * r + t * f;
	if (otherRigid != nullptr) {
		otherRigid->acceleration = vec2f(0.f);
		otherRigid->velocity = otherRigid->velocity + (p * r + t * f);
	}
	// Move the rigid & its collider to avoid overlapping.
	transform->translate(separation);
}

};