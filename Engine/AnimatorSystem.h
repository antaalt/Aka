#pragma once

#include "System.h"
#include "Animator.h"

namespace app {

class AnimatorSystem: public System
{
public:
	AnimatorSystem(World* world);
public:
	void update() override;
};


}

