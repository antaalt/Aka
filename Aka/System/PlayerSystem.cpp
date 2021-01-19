#include "PlayerSystem.h"

#include "../Platform/Logger.h"
#include "../Core/World.h"
#include "../Component/Transform2D.h"
#include "../Component/Interact.h"
#include "../Component/Player.h"
#include "../Component/Animator.h"

namespace aka {

PlayerSystem::PlayerSystem(World* world) :
	System(world)
{
}

void PlayerSystem::create()
{
	m_world->subscribe<CollisionEvent>(this);
}

void PlayerSystem::destroy()
{
	m_world->unsubscribe<CollisionEvent>(this);
}

void PlayerSystem::update(Time::Unit deltaTime)
{
	// Inputs for player, find the player component ?
}

void PlayerSystem::receive(World* world, const CollisionEvent& collision)
{
	Player* player = collision.left->get<Player>();
	if (player == nullptr)
		return;
	Coin* coin = collision.right->get<Coin>();
	if (coin != nullptr)
	{
		if (!coin->picked)
		{
			player->coin++;
			coin->picked = true;
			collision.right->get<Animator>()->play("picked");
		}
	}
	else
	{
		RigidBody2D* rigid = collision.left->get<RigidBody2D>();
		Transform2D* transform = collision.left->get<Transform2D>();
		Collider2D* collider = collision.left->get<Collider2D>();
		Collider2D* otherCollider = collision.right->get<Collider2D>();

		// Adjust velocity
		// Get normal 
		vec2f normal = vec2f::normalize(collision.separation);
		// Get relative velocity
		vec2f v = rigid->velocity; // substract by second object velocity if it has one
		// Get penetration speed
		float ps = vec2f::dot(v, normal);
		// objects moving towards each other ?
		if (ps <= 0.f)
		{
			// Move the rigid & its collider to avoid overlapping.
			transform->position += collision.separation;
		}
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
	}
}

};