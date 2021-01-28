#include "SoundSystem.h"

#include "../../OS/Logger.h"
#include "../../Core/ECS/World.h"

namespace aka {


SoundSystem::SoundSystem(World* world) :
	System(world),
	m_player(44100, 2)
{
}

void SoundSystem::update(Time::Unit deltaTime)
{
	m_world->each<SoundInstance>([&](Entity * entity, SoundInstance * sound) {
		if (m_player.exist(sound->decoder))
		{
			m_player.setVolume(sound->decoder, sound->volume);
			if (m_player.finished(sound->decoder))
			{
				m_player.close(sound->decoder);
				entity->destroy();
			}
		}
		else
		{
			sound->decoder = m_player.play(sound->path, sound->loop);
		}
	});
}

};