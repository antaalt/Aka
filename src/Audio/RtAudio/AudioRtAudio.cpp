#include "AudioRtAudio.h"

#include <Aka/OS/Logger.h>

#if defined(AKA_PLATFORM_WINDOWS)
#pragma comment(lib, "dsound.lib")
#endif

namespace aka {

AudioRtAudio::AudioRtAudio(const AudioConfig& config) :
	AudioDevice(config)
{
#if defined(AKA_PLATFORM_WINDOWS)
	m_audio = new RtAudio(RtAudio::Api::WINDOWS_DS);
#else
	m_audio = new RtAudio(RtAudio::Api::LINUX_PULSE);
#endif
	// Determine the number of devices available
	unsigned int devices = m_audio->getDeviceCount();
	if (devices == 0)
		throw std::runtime_error("No audio devices found");

	RtAudio::StreamParameters parameters;
	parameters.deviceId = m_audio->getDefaultOutputDevice();
	parameters.nChannels = m_channelCount;
	parameters.firstChannel = 0;

	// Frames is not samples
	// One frame is equivalent to samples for all the channels
	// Sample = channel * frames
	uint32_t frames = 2048;
	try
	{
		m_audio->openStream(
			&parameters,
			nullptr,
			RTAUDIO_SINT16,
			m_frequency,
			&frames,
			[](
				void* outputBuffer,
				void* inputBuffer,
				unsigned int nFrames,
				double streamTime,
				RtAudioStreamStatus status,
				void* userData
				)
			{
				AudioFrame* out = static_cast<AudioFrame*>(outputBuffer);
				if (status) {
					if (status & RTAUDIO_INPUT_OVERFLOW) {
						Logger::warn("Audio stream overflow detected!");
					}
					if (status & RTAUDIO_OUTPUT_UNDERFLOW) {
						Logger::warn("Audio stream underflow detected!");
					}
				}
				AudioRtAudio* audio = static_cast<AudioRtAudio*>(userData);
				audio->process(out, nFrames);
				return 0;
			},
			this
		);
		m_audio->startStream();
	}
	catch (RtAudioError& e)
	{
		Logger::error("[Rtaudio]", e.getMessage());
		throw;
	}
}

AudioRtAudio::~AudioRtAudio()
{
	if (m_audio->isStreamRunning())
		m_audio->stopStream();
	if (m_audio->isStreamOpen())
		m_audio->closeStream();
	delete m_audio;
}

bool AudioRtAudio::play(AudioStream::Ptr stream)
{
	AKA_ASSERT(m_frequency == stream->frequency(), "Audio will need resampling");
	AKA_ASSERT(m_channelCount == stream->channels(), "Audio channels does not match");
	std::lock_guard<std::mutex> m(m_lock);
	auto it = m_streams.insert(stream);
	return it.second;
}

void AudioRtAudio::close(AudioStream::Ptr stream)
{
	std::lock_guard<std::mutex> m(m_lock);
	auto it = m_streams.find(stream);
	if (it != m_streams.end())
		m_streams.erase(it);
}

bool AudioRtAudio::playing(AudioStream::Ptr stream)
{
	std::lock_guard<std::mutex> m(m_lock);
	auto it = m_streams.find(stream);
	return (it != m_streams.end());
}

AudioFrame mix(AudioFrame sample1, AudioFrame sample2)
{
	const int32_t result(static_cast<int32_t>(sample1) + static_cast<int32_t>(sample2));
	using range = std::numeric_limits<AudioFrame>;
	if ((range::max)() < result)
		return (range::max)();
	else if ((range::min)() > result)
		return (range::min)();
	else
		return result;
}

void AudioRtAudio::process(AudioFrame* buffer, uint32_t frames)
{
	std::lock_guard<std::mutex> m(m_lock);
	// Set buffer to zero for mixing
	memset(buffer, 0, frames * m_channelCount * sizeof(AudioFrame));
	if (m_streams.size() == 0)
		return; // No audio to process
	// Mix all audiodecoder
	std::vector<AudioFrame> tmp(frames * m_channelCount);
	for (auto it = m_streams.begin(); it != m_streams.end();)
	{
		std::set<AudioStream::Ptr>::iterator current = it++;
		AudioStream* stream = (*current).get();
		if (!stream->decode(tmp.data(), frames * m_channelCount))
			m_streams.erase(current);
		for (unsigned int i = 0; i < frames * m_channelCount; i++)
			buffer[i] = mix(buffer[i], static_cast<AudioFrame>(tmp[i] * stream->volume()));
	}
}

};
