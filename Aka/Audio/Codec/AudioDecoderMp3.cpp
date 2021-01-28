#include "AudioDecoderMp3.h"


//#define MINIMP3_ONLY_MP3
//#define MINIMP3_ONLY_SIMD
//#define MINIMP3_NO_SIMD
//#define MINIMP3_NONSTANDARD_BUT_LOGICAL
//#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_IMPLEMENTATION
#include <minimp3.h>
#include <minimp3_ex.h>

#include "../../OS/Logger.h"
#include "../../Core/Debug.h"

namespace aka {


AudioDecoderMp3::AudioDecoderMp3(const Path& path, float volume, bool loop) :
    AudioDecoder(volume, loop)
{
    memset(&m_mp3d, 0, sizeof(m_mp3d));
    int sample = mp3dec_ex_open(&m_mp3d, path.c_str(), MP3D_SEEK_TO_SAMPLE);
    if (!m_mp3d.samples)
        Logger::error("Could not load audio");
    else
        m_playing = true;
}

AudioDecoderMp3::~AudioDecoderMp3()
{
    mp3dec_ex_close(&m_mp3d);
    memset(&m_mp3d, 0, sizeof(m_mp3d));
}

void AudioDecoderMp3::decode(int16_t* buffer, uint32_t bytes)
{
    memset(buffer, 0, bytes);
    size_t readed = mp3dec_ex_read(&m_mp3d, (mp3d_sample_t*)buffer, bytes);
    ASSERT(m_mp3d.last_error == 0, "Error while reading file");
    if (readed != bytes)
    {
        // Reached eof
        if (m_loop)
            seek(0);
        else
            m_playing = false;
    }
}

void AudioDecoderMp3::seek(uint64_t position)
{
    mp3dec_ex_seek(&m_mp3d, position);
}

uint32_t AudioDecoderMp3::channels() const
{
    return static_cast<uint32_t>(m_mp3d.info.channels);
}

uint32_t AudioDecoderMp3::frequency() const
{
    return static_cast<uint32_t>(m_mp3d.info.hz);
}

uint64_t AudioDecoderMp3::samples() const
{
    return m_mp3d.samples;
}

size_t AudioDecoderMp3::fileSize() const
{
    return m_mp3d.file.size;
}

};