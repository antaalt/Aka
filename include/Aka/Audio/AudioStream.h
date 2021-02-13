#pragma once

#include <memory>

#include <Aka/OS/FileSystem.h>
#include <Aka/Audio/Audio.h>

namespace aka {

class AudioStream
{
public:
    using Ptr = std::shared_ptr<AudioStream>;
public:
    // Open an audio stream from file and return it.
    static AudioStream::Ptr openStream(const Path& path);
    // Open an audio from file and return it. Load the whole audio in memory.
    static AudioStream::Ptr loadMemory(const Path& path);

public:
    AudioStream(uint32_t frequency, uint32_t channels) : m_frequency(frequency), m_channels(channels) {}
    // Decode an audio file and return a memory buffer
    virtual bool load(const Path& path, Audio* audio) const = 0;
    // Open an audio file and return a stream
    virtual bool open(const Path& path) = 0;
    // Close an audio stream
    virtual bool close() = 0;
    // Decode bytes (frame * channel) elements in buffer
    virtual bool decode(AudioFrame* buffer, size_t bytes) = 0;
    // Seek in the stream
    virtual void seek(uint64_t position) = 0;
    // Is the stream playing
    virtual bool playing() const = 0;
    // Get frequency
    uint32_t frequency() const { return m_frequency; }
    // Get channel count
    uint32_t channels() const { return m_channels; }
protected:
    uint32_t m_frequency;
    uint32_t m_channels;
};

};