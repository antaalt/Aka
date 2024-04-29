#pragma once

#include <Aka/Audio/AudioStream.h>

namespace aka {

class AudioStreamMemory : public AudioStream
{
public:
    AudioStreamMemory(const Vector<AudioFrame>& data, uint32_t frequency, uint32_t channels);
    AudioStreamMemory(const AudioFrame* data, size_t length, uint32_t frequency, uint32_t channels);

    bool load(const Path& path, Audio *audio) const override;
    bool open(const Path& path) override;
    bool close() override;
    bool decode(AudioFrame* buffer, size_t bytes) override;
    void seek(uint64_t position) override;
    bool playing() const override;
    uint64_t offset() const override;
    uint32_t frequency() const override;
    uint32_t channels() const override;
    uint64_t samples() const override;
private:
    Vector<AudioFrame> m_frames;
    size_t m_offset;
    uint32_t m_frequency;
    uint32_t m_channels;
};

};