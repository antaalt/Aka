#pragma once

#include <stdint.h>
#include <set>

#include <Aka/Audio/AudioStream.h>

namespace aka {

class AudioDevice
{
public:
	AudioDevice(uint32_t frequency, uint32_t channels);
	virtual ~AudioDevice();

	// Get the frequency of the device
	uint32_t getFrequency();
	// Get the channel count of the device
	uint32_t getChannelCount();

	// Play an audio at path.
	virtual bool play(AudioStream::Ptr stream) = 0;
	// Close an audio
	virtual void close(AudioStream::Ptr stream) = 0;
	// Is an audio currently playing
	virtual bool playing(AudioStream::Ptr stream) = 0;
protected:
	// Process the audio
	virtual void process(AudioFrame* buffer, uint32_t frames) = 0;
protected:
	uint32_t m_frequency;
	uint32_t m_channelCount;
	std::set<AudioStream::Ptr> m_streams;
};

class AudioBackend
{
public:
	static void initialize(uint32_t frequency, uint32_t channels);
	static void destroy();
	static AudioDevice* get();

};

};