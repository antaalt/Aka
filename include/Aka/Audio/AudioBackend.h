#pragma once

#include <stdint.h>

#include <Aka/Audio/AudioStream.h>

namespace aka {

class AudioBackend
{
public:
	// Initialize the audio device
	static void initialize(uint32_t frequency, uint32_t channels);
	// Destroy the audio device
	static void destroy();
	// Start the audio backend
	static void start();
	// Stop the audio backend
	static void stop();
	// Get the number of audio device
	static uint32_t getDeviceCount();
	// Get the frequency of the backend
	static uint32_t getFrequency();
	// Get the channels of the backend
	static uint32_t getChannels();

	// Play an audio at path.
	static bool play(AudioStream::Ptr stream, float volume, bool loop = false);
	// Close an audio
	static void close(AudioStream::Ptr stream);
private:
	// Process the audio
	static void process(AudioFrame* buffer, uint32_t frames);
};

};