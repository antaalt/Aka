#include "CoinSystem.h"

#include "../Core/World.h"
#include "../Component/Interact.h"
#include "../Component/Animator.h"
#include "../Platform/Logger.h"

namespace aka {

CoinSystem::CoinSystem(World* world) :
	System(world)
{
}

void CoinSystem::create()
{
	m_world->subscribe<AnimationFinishedEvent>(this);
}

void CoinSystem::destroy()
{
	m_world->unsubscribe<AnimationFinishedEvent>(this);
}

void CoinSystem::update(Time::Unit deltaTime)
{
}

void CoinSystem::receive(World* world, const AnimationFinishedEvent& event)
{
	Coin* coin = event.entity->get<Coin>();
	if (coin == nullptr || !coin->picked)
		return;
	event.entity->destroy();
}

}