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
    AudioStream() : m_volume(1.f) {}
    virtual ~AudioStream() {}
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
    // Get current sample offset
    virtual uint64_t offset() const = 0;
    // Get frequency
    virtual uint32_t frequency() const = 0;
    // Get channel count
    virtual uint32_t channels() const = 0;
    // Get samples count
    virtual uint64_t samples() const = 0;
	// Get volume
	float volume() const { return m_volume; }
	// Set volume
	void setVolume(float volume) { m_volume = volume; }
protected:
	float m_volume;
};

};