#pragma once

#include "../Core/System.h"
#include "../Component/Collider2D.h"

namespace aka {

struct CollisionEvent {
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
