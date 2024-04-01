#pragma once

#include <Aka/Audio/AudioDevice.h>

#include <mutex>
#include <RtAudio.h>

namespace aka {

class RtAudioDevice : public AudioDevice
{
public:
	RtAudioDevice(const AudioConfig& config);
	~RtAudioDevice();

	void initialize(const AudioConfig& config) override;
	void shutdown() override;

	// Play an audio at path.
	bool play(AudioStream* stream) override;
	// Close an audio
	void close(AudioStream* stream) override;
	// Is an audio currently playing
	bool playing(AudioStream* stream) override;
protected:
	// Process the audio
	void process(AudioFrame* buffer, uint32_t frames) override;
protected:
	RtAudio* m_audio;
	std::mutex m_lock;
};

};