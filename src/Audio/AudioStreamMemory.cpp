#include <Aka/Audio/AudioStreamMemory.h>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Config.h>
#include <Aka/Core/Geometry.h>

#include <cstring>

namespace aka {

AudioStreamMemory::AudioStreamMemory(const Vector<AudioFrame>& data, uint32_t frequency, uint32_t channels) :
    AudioStreamMemory(data.data(), data.size(), frequency, channels)
{
}

AudioStreamMemory::AudioStreamMemory(const AudioFrame* data, size_t length, uint32_t frequency, uint32_t channels) :
    m_frames(data, length),
    m_offset(0),
    m_frequency(frequency),
    m_channels(channels)
{
}

bool AudioStreamMemory::load(const Path& path, Audio *audio) const
{
	AKA_ASSERT(false, "Cannot load an memory stream directly");
    return false;
}

bool AudioStreamMemory::open(const Path& path)
{
	AKA_ASSERT(false, "Cannot open an memory stream");
    return false;
}

bool AudioStreamMemory::close()
{
	AKA_ASSERT(false, "Cannot close an memory stream");
    return false;
}

bool AudioStreamMemory::decode(AudioFrame* buffer, size_t bytes)
{
    size_t copySize = min(bytes, m_frames.size() - m_offset);
    memcpy(buffer, m_frames.data() + m_offset, copySize * sizeof(AudioFrame));
    m_offset += copySize;
    if (copySize != bytes) // Reached eof
        return false;
    return true;
}
void AudioStreamMemory::seek(uint64_t position)
{
    m_offset = position * m_channels;
}

bool AudioStreamMemory::playing() const
{
    return m_offset < m_frames.size();
}

uint64_t AudioStreamMemory::offset() const
{
    return m_offset / m_channels;
}

uint32_t AudioStreamMemory::frequency() const
{
    return m_frequency;
}

uint32_t AudioStreamMemory::channels() const
{
    return m_channels;
}

uint64_t AudioStreamMemory::samples() const
{
    return m_frames.size() / m_channels;
}

};