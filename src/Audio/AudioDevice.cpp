#include <Aka/Audio/AudioDevice.h>

#include "RtAudio/AudioRtAudio.h"

namespace aka {

static AudioDevice* s_audio;

void AudioBackend::initialize(uint32_t frequency, uint32_t channels)
{
	s_audio = new AudioRtAudio(frequency, channels);
}

void AudioBackend::destroy()
{
	delete s_audio;
	s_audio = nullptr;
}

AudioDevice* AudioBackend::get()
{
	return s_audio;
}

AudioDevice::AudioDevice(uint32_t frequency, uint32_t channels) :
	m_frequency(frequency),
	m_channelCount(channels)
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