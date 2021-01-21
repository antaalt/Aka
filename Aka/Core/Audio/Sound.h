#pragma once

#include "../../Platform/IO/FileSystem.h"

#include <minimp3.h>
#include <minimp3_ex.h>
#include <RtAudio.h>

namespace aka {

struct SoundPlayer {
    SoundPlayer();

    void create();

    void destroy();
private:
    RtAudio m_audio;
};

struct AudioDecoder
{
    AudioDecoder(const Path &path);
	~AudioDecoder();

    static AudioDecoder create(const Path& path);



    void decode(int16_t *buffer, uint32_t bytes);
    void seek();
    uint32_t channels() const;
    uint32_t frequency() const;
    uint64_t samples() const;
private:
    mp3dec_ex_t m_mp3d;
};

};