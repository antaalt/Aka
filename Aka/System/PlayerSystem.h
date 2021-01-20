#pragma once

#include "../Core/System.h"
#include "../Core/Event.h"
#include "CollisionSystem.h"

namespace aka {

class PlayerSystem : public System
{
public:
	PlayerSystem(World* world);

	void update(Time::Unit deltaTime) override;
};

};