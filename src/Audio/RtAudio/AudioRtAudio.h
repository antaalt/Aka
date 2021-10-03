#pragma once

#include <Aka/Audio/AudioDevice.h>

#include <mutex>
#include <RtAudio.h>

namespace aka {

class AudioRtAudio : public AudioDevice
{
public:
	AudioRtAudio(const AudioConfig& config);
	~AudioRtAudio();

	// Play an audio at path.
	bool play(AudioStream::Ptr stream) override;
	// Close an audio
	void close(AudioStream::Ptr stream) override;
	// Is an audio currently playing
	bool playing(AudioStream::Ptr stream) override;
protected:
	// Process the audio
	void process(AudioFrame* buffer, uint32_t frames) override;
protected:
	RtAudio* m_audio;
	std::mutex m_lock;
};

};