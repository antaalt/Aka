#pragma once

#include "../Platform/IO/FileSystem.h"
#include "../Core/System.h"
#include "../Core/Audio/Sound.h"

#include <RtAudio.h>

namespace aka {

struct SoundInstance : public Component {
	SoundInstance() {}
	SoundInstance(const Path &path, bool loop = false) : path(path), loop(loop) {}
	Path path;
	bool loop;
};

class SoundSystem : public System
{
public:
	SoundSystem(World* world);

	void update(Time::Unit deltaTime) override;
private:
	std::map<SoundInstance*, std::unique_ptr<SoundPlayer>> m_sounds;
};

};
