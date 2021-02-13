#pragma once

#include <Aka/Audio/AudioStream.h>

#include <minimp3.h>
#include <minimp3_ex.h>

namespace aka {

class AudioStreamMp3 : public AudioStream
{
public:
    AudioStreamMp3();

    bool load(const Path& path, Audio* audio) const override;
    bool open(const Path& path) override;
    bool close() override;
    bool decode(AudioFrame* buffer, size_t bytes) override;
    void seek(uint64_t position) override;
    bool playing() const override;
private:
    mp3dec_ex_t m_mp3d;
};

};