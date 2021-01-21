#include "CoinSystem.h"

#include "../Core/World.h"
#include "../Component/Coin.h"
#include "../Component/Animator.h"
#include "../Platform/Logger.h"

namespace aka {

CoinSystem::CoinSystem(World* world) :
	System(world)
{
}

void CoinSystem::update(Time::Unit deltaTime)
{
	m_world->receive<AnimationFinishedEvent>([](AnimationFinishedEvent* event) {
		Coin* coin = event->entity->get<Coin>();
		if (coin == nullptr || !coin->picked)
			return;
		event->entity->destroy();
	});
}

}