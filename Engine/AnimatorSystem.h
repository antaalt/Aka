#pragma once

#include "System.h"
#include "Animator.h"

namespace app {

class AnimatorSystem: public System
{
public:
	void update() override;
public:
	bool valid(Entity* entity) override;
};


}

