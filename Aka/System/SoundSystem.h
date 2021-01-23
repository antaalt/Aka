#pragma once

#include "../Platform/IO/FileSystem.h"
#include "../Core/ECS/System.h"
#include "../Core/Audio/AudioPlayer.h"

#include <RtAudio.h>

namespace aka {

class SoundSystem;

struct SoundInstance : public Component {
	friend class SoundSystem;
	SoundInstance() : SoundInstance("", 1.f, false) {}
	SoundInstance(const Path &path, float volume, bool loop = false) : path(path), volume(volume), loop(loop) {}

	Path path;
	float volume;
	bool loop;
private:
	AudioDecoder::ID decoder;
};

class SoundSystem : public System
{
public:
	SoundSystem(World* world);

	void update(Time::Unit deltaTime) override;
private:
	AudioPlayer m_player;
};

};
