#include <Aka/Audio/AudioStreamMp3.h>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Debug.h>

//#define MINIMP3_ONLY_MP3
//#define MINIMP3_ONLY_SIMD
//#define MINIMP3_NO_SIMD
//#define MINIMP3_NONSTANDARD_BUT_LOGICAL
//#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_IMPLEMENTATION

#include <minimp3.h>
#include <minimp3_ex.h>

#include <utf8.h>


namespace aka {

AudioStreamMp3::AudioStreamMp3() :
    m_mp3d{}
{
}
AudioStreamMp3::~AudioStreamMp3()
{
    close();
}

bool AudioStreamMp3::load(const Path& path, Audio* audio) const
{
    std::wstring wstr;
    utf8::utf8to16(path.str().begin(), path.str().end(), std::back_inserter(wstr));
    mp3dec_t mp3d{};
    mp3dec_file_info_t info{};
    int sample = mp3dec_load_w(&mp3d, wstr.c_str(), &info, nullptr, nullptr);
    if (sample != 0)
        return false;
    ASSERT(sizeof(AudioFrame) == sizeof(mp3d_sample_t), "Incorrect size");
    audio->frames.resize(info.samples);
    audio->channels = info.channels;
    audio->duration = info.samples / (float)(info.channels * info.hz);
    audio->frequency = info.hz;
    memcpy(audio->frames.data(), info.buffer, info.samples * sizeof(mp3d_sample_t));
    free(info.buffer);
    return true;
}

bool AudioStreamMp3::open(const Path& path)
{
    memset(&m_mp3d, 0, sizeof(m_mp3d));
    std::wstring wstr;
    utf8::utf8to16(path.str().begin(), path.str().end(), std::back_inserter(wstr));
    int sample = mp3dec_ex_open_w(&m_mp3d, wstr.c_str(), MP3D_SEEK_TO_SAMPLE);
    if (m_mp3d.samples > 0)
        return true;
    return false;
}

bool AudioStreamMp3::close()
{
    mp3dec_ex_close(&m_mp3d);
    return true;
}

bool AudioStreamMp3::decode(AudioFrame* buffer, size_t bytes)
{
    size_t readed = mp3dec_ex_read(&m_mp3d, (mp3d_sample_t*)buffer, bytes);
    ASSERT(m_mp3d.last_error == 0, "Error while reading file");
    if (readed != bytes) // Reached eof
        return false;
    return true;
}
void AudioStreamMp3::seek(uint64_t position)
{
    int result = mp3dec_ex_seek(&m_mp3d, position * m_mp3d.info.channels);
    ASSERT(result == 0, "Error while seeking");
}

bool AudioStreamMp3::playing() const
{
    return m_mp3d.cur_sample < m_mp3d.samples;
}

uint32_t AudioStreamMp3::offset() const
{
    return m_mp3d.cur_sample / m_mp3d.info.channels;
}

uint32_t AudioStreamMp3::frequency() const
{
    return (uint32_t)m_mp3d.info.hz;
}

uint32_t AudioStreamMp3::channels() const
{
    return (uint32_t)m_mp3d.info.channels;
}

uint64_t AudioStreamMp3::samples() const
{
    return m_mp3d.samples / m_mp3d.info.channels;
}

};