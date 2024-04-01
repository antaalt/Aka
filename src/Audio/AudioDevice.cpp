#include <Aka/Audio/AudioDevice.h>

#include "RtAudio/RtAudioDevice.h"

namespace aka {


AudioDevice* AudioDevice::create(const AudioConfig& config)
{
#if defined(AKA_USE_RTAUDIO)
	return new RtAudioDevice(config);
#else
	return new DummyAudioDevice(config);
#endif
}

void AudioDevice::destroy(AudioDevice* device)
{
	delete device;
}


AudioDevice::AudioDevice(const AudioConfig& config) :
	m_frequency(config.frequency),
	m_channelCount(config.channels)
{

}

AudioDevice::~AudioDevice()
{

}

uint32_t AudioDevice::getFrequency()
{
	return m_frequency;
}

uint32_t AudioDevice::getChannelCount()
{
	return m_channelCount;
}

};