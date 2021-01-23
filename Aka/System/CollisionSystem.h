#pragma once

#include "../Core/ECS/System.h"
#include "../Core/ECS/Event.h"
#include "../Component/Collider2D.h"

namespace aka {

struct CollisionEvent : public Event {
	CollisionEvent(Entity *left, Entity *right, vec2f separation) :
		left(left), right(right), separation(separation) {}

	Entity* left;
	Entity* right;
	vec2f separation;

	void resolve() const;
};

class CollisionSystem : public System
{
public:
	CollisionSystem(World* world);

	void update(Time::Unit deltaTime) override;
};

};
