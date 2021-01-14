#pragma once

#include "System.h"
#include "Animator.h"

namespace aka {

class AnimatorSystem: public System
{
public:
	AnimatorSystem(World* world);
public:
	void update() override;
};


}

