#include "SoundSystem.h"

#include "../Platform/Logger.h"
#include "../Core/World.h"

namespace aka {


SoundSystem::SoundSystem(World* world) :
	System(world)
{
}

void SoundSystem::update(Time::Unit deltaTime)
{
    // Manage audiodecoder depending on sound to play
	m_world->each<SoundInstance>([&](Entity * entity, SoundInstance * sound) {
		auto it = m_sounds.find(sound);
		if (it == m_sounds.end())
		{
			// Start the sound
			m_sounds.insert(std::make_pair(sound, std::make_unique<SoundPlayer>(sound->path)));
		}
		else
		{
			// Stop the sound
			if (!it->second->playing())
			{
				m_sounds.erase(it);
				entity->destroy();
			}
		}
	});
}

};