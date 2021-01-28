#pragma once

#include <minimp3.h>
#include <minimp3_ex.h>

#include "AudioDecoder.h"

namespace aka {

struct AudioDecoderMp3 : public AudioDecoder
{
    AudioDecoderMp3(const Path& path, float volume, bool loop = false);
    ~AudioDecoderMp3();

    void decode(int16_t* buffer, uint32_t bytes) override;
    void seek(uint64_t position) override;
    uint32_t channels() const override;
    uint32_t frequency() const override;
    uint64_t samples() const override;
    size_t fileSize() const override;
private:
    mp3dec_ex_t m_mp3d;
};


};