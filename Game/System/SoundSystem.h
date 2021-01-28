#pragma once

#include <OS/FileSystem.h>
#include <Core/ECS/System.h>
#include <Audio/AudioBackend.h>

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
	AudioID audio;
};

class SoundSystem : public System
{
public:
	SoundSystem(World* world);

	void update(Time::Unit deltaTime) override;
};

};
