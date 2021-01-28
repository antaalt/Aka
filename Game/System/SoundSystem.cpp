#include "SoundSystem.h"

#include <OS/Logger.h>
#include <Core/ECS/World.h>

namespace aka {


SoundSystem::SoundSystem(World* world) :
	System(world)
{
}

void SoundSystem::update(Time::Unit deltaTime)
{
	m_world->each<SoundInstance>([&](Entity * entity, SoundInstance * sound) {
		if (AudioBackend::exist(sound->audio))
		{
			AudioBackend::setVolume(sound->audio, sound->volume);
			if (AudioBackend::finished(sound->audio))
			{
				AudioBackend::close(sound->audio);
				entity->destroy();
			}
		}
		else
		{
			sound->audio = AudioBackend::play(sound->path, sound->loop);
		}
	});
}

};