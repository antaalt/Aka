#pragma once

#include <Aka/Audio/AudioStream.h>

namespace aka {

class AudioStreamMemory : public AudioStream
{
public:
    AudioStreamMemory(const std::vector<AudioFrame>& data, uint32_t frequency, uint32_t channels);
    AudioStreamMemory(const AudioFrame* data, size_t length, uint32_t frequency, uint32_t channels);

    bool load(const Path& path, Audio *audio) const override;
    bool open(const Path& path) override;
    bool close() override;
    bool decode(AudioFrame* buffer, size_t bytes) override;
    void seek(uint64_t position) override;
    bool playing() const override;
private:
    std::vector<AudioFrame> m_frames;
    size_t m_offset;
};

};