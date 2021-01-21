#include "SoundSystem.h"

#include "../Platform/Logger.h"
#include "../Core/Audio/Sound.h"

namespace aka {


SoundSystem::SoundSystem(World* world) :
	System(world)
{
}

static SoundPlayer player;

void SoundSystem::create()
{
    player.create();
}

void SoundSystem::destroy()
{
    player.destroy();
}

void SoundSystem::update(Time::Unit deltaTime)
{
}

};