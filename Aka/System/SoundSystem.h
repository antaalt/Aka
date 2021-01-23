#pragma once

#include "../Platform/IO/FileSystem.h"
#include "../Core/ECS/System.h"
#include "../Core/Audio/AudioPlayer.h"

#include <RtAudio.h>

namespace aka {

class SoundSystem;

struct SoundInstance : public Component {
	friend class SoundSystem;
	SoundInstance(): SoundInstance("", false) {}
	SoundInstance(const Path &path, bool loop = false) : path(path), loop(loop) {}

	Path path;
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
