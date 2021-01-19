#pragma once

#include "../Core/System.h"
#include "../Component/Collider2D.h"

namespace aka {

class CollisionSystem : public System
{
public:
	CollisionSystem(World* world);

	void update(Time::Unit deltaTime) override;
};

};
