#include <Aka/Audio/AudioStreamMemory.h>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Debug.h>
#include <Aka/Core/Geometry.h>

namespace aka {

AudioStreamMemory::AudioStreamMemory(const std::vector<AudioFrame>& data, uint32_t frequency, uint32_t channels) :
    AudioStreamMemory(data.data(), data.size(), frequency, channels)
{
}

AudioStreamMemory::AudioStreamMemory(const AudioFrame* data, size_t length, uint32_t frequency, uint32_t channels) :
    m_frames(length),
    m_offset(0),
    m_frequency(frequency),
    m_channels(channels)
{
    memcpy(m_frames.data(), data, length);
}

bool AudioStreamMemory::load(const Path& path, Audio *audio) const
{
    ASSERT(false, "Cannot load an memory stream directly");
    return false;
}

bool AudioStreamMemory::open(const Path& path)
{
    ASSERT(false, "Cannot open an memory stream");
    return false;
}

bool AudioStreamMemory::close()
{
    ASSERT(false, "Cannot close an memory stream");
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
    m_offset = position;
}

bool AudioStreamMemory::playing() const
{
    return m_offset < m_frames.size();
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