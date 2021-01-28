#pragma once

#include <Core/ECS/System.h>
#include <Core/ECS/Event.h>
#include "CollisionSystem.h"

namespace aka {

class PlayerSystem : public System
{
public:
	PlayerSystem(World* world);

	void update(Time::Unit deltaTime) override;
};

};