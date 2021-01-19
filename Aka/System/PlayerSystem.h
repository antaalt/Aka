#pragma once

#include "../Core/System.h"
#include "../Core/Event.h"
#include "CollisionSystem.h"

namespace aka {

class PlayerSystem : public System, EventSubscriber<CollisionEvent>
{
public:
	PlayerSystem(World* world);

	void create();
	void destroy();

	void update(Time::Unit deltaTime) override;

	void receive(World* world, const CollisionEvent& event) override;
};

};