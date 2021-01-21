#pragma once

#include "../Core/System.h"
#include "../Core/Event.h"
#include "CollisionSystem.h"

#include <RtAudio.h>

namespace aka {



class SoundSystem : public System
{
public:
	SoundSystem(World* world);

	void create() override;

	void destroy() override;

	void update(Time::Unit deltaTime) override;
};

};
