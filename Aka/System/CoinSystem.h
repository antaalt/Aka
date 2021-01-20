#pragma once
#include "../Core/System.h"
#include "../Core/Event.h"
#include "../Component/Animator.h"
#include "../System/AnimatorSystem.h"

namespace aka {

class CoinSystem : public System
{
public:
	CoinSystem(World* world);
public:
	void update(Time::Unit deltaTime) override;
};


};

