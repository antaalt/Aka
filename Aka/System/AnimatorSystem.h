#pragma once

#include "../Core/System.h"
#include "../Component/Animator.h"

namespace aka {

struct AnimationFinishedEvent {
	Entity* entity;
};

class AnimatorSystem: public System
{
public:
	AnimatorSystem(World* world);
public:
	void update(Time::Unit deltaTime) override;
};


}

