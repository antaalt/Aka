#pragma once

#include "../../Platform/IO/FileSystem.h"

#include <minimp3.h>
#include <minimp3_ex.h>
#include <RtAudio.h>

namespace aka {

struct Mp3AudioDecoder
{
    Mp3AudioDecoder(const Path &path);
	~Mp3AudioDecoder();

    bool decode(int16_t *buffer, uint32_t bytes);
    void seek();
    uint32_t channels() const;
    uint32_t frequency() const;
    uint64_t samples() const;
    size_t fileSize() const;
private:
    mp3dec_ex_t m_mp3d;
};

struct SoundPlayer {
    SoundPlayer(const Path& path);
    ~SoundPlayer();

    bool playing();

    const Mp3AudioDecoder& decoder() const;

private:
    Mp3AudioDecoder m_decoder;
    RtAudio m_audio;
};

};