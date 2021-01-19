#pragma once
#include "../Core/System.h"
#include "../Core/Event.h"
#include "../Component/Animator.h"
#include "../System/AnimatorSystem.h"

namespace aka {

class CoinSystem : public System, public EventSubscriber<AnimationFinishedEvent>
{
public:
	CoinSystem(World* world);
public:
	void create() override;
	void destroy() override;
	void update(Time::Unit deltaTime) override;
	void receive(World* world, const AnimationFinishedEvent& event) override;
};


};

