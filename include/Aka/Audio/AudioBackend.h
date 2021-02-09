#pragma once

#include <stdint.h>

#include "../OS/FileSystem.h"
#include "../Core/StrictType.h"

namespace aka {

using AudioID = StrictType<uintptr_t, struct AudioTagName>;

class AudioBackend
{
public:
	// Initialize the audio device
	static void initialize(uint32_t frequency, uint32_t channels);
	// Destroy the audio device
	static void destroy();
	// Get the number of audio device
	static uint32_t getDeviceCount();
	// Get the frequency of the backend
	static uint32_t getFrequency();
	// Get the channels of the backend
	static uint32_t getChannels();

	// Play an audio at path.
	static AudioID play(const Path& path, float volume, bool loop = false);
	// Check if an audio has finished playing
	static bool finished(AudioID id);
	// Check if an audio exist
	static bool exist(AudioID id);
	// Set the volume of an audio
	static void setVolume(AudioID id, float volume);
	// Close an audio
	static void close(AudioID id);
private:
	// Process the audio
	static void process(int16_t* buffer, uint32_t frames);
};

};