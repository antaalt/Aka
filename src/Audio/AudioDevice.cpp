#include <Aka/Audio/AudioDevice.h>

#include "RtAudio/AudioRtAudio.h"

namespace aka {

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