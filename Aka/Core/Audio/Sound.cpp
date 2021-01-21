#include "Sound.h"

//#define MINIMP3_ONLY_MP3
//#define MINIMP3_ONLY_SIMD
//#define MINIMP3_NO_SIMD
//#define MINIMP3_NONSTANDARD_BUT_LOGICAL
//#define MINIMP3_FLOAT_OUTPUT

#define MINIMP3_IMPLEMENTATION
#include <minimp3.h>
#include <minimp3_ex.h>
#include <vector>

#include "../../Platform/Logger.h"
#include "../../Core/Debug.h"


namespace aka {

AudioDecoder::AudioDecoder(const Path& path)
{
    memset(&m_mp3d, 0, sizeof(m_mp3d));
    int sample = mp3dec_ex_open(&m_mp3d, path.c_str(), MP3D_SEEK_TO_SAMPLE);
    if (!m_mp3d.samples)
        Logger::error("Could not load audio");
}

AudioDecoder::~AudioDecoder()
{
    mp3dec_ex_close(&m_mp3d);
    memset(&m_mp3d, 0, sizeof(m_mp3d));
}

void AudioDecoder::decode(int16_t* buffer, uint32_t bytes)
{
    memset(buffer, 0, bytes);
    size_t out = mp3dec_ex_read(&m_mp3d, (mp3d_sample_t*)buffer, bytes);
}

void AudioDecoder::seek()
{
   // mp3dec_ex_seek(&_dec.mp3d, progress);
}

uint32_t AudioDecoder::channels() const
{
    return static_cast<uint32_t>(m_mp3d.info.channels);
}

uint32_t AudioDecoder::frequency() const
{
    return static_cast<uint32_t>(m_mp3d.info.hz);
}

uint64_t AudioDecoder::samples() const
{
    return m_mp3d.samples;
}

static AudioDecoder decoder(Asset::path("sounds/forest.mp3"));

SoundPlayer::SoundPlayer() :
    m_audio(RtAudio::Api::WINDOWS_DS)
{
}

void SoundPlayer::create()
{
    // Determine the number of devices available
    unsigned int devices = m_audio.getDeviceCount();
    if (devices == 0)
        throw std::runtime_error("No audio devices found");
    // Scan through devices for various capabilities
    RtAudio::DeviceInfo info;
    for (unsigned int i = 0; i < devices; i++) {
        info = m_audio.getDeviceInfo(i);
        if (info.probed == true) {
            Logger::info("Audio device ", i, " : ", info.name);
        }
    }

    RtAudio::StreamParameters parameters;
    parameters.deviceId = m_audio.getDefaultOutputDevice();
    parameters.nChannels = decoder.channels();
    parameters.firstChannel = 0;

    // Frames is not samples
    // One frame is equivalent to samples for all the channels
    // Sample = channel * frames
    uint32_t frames = 2048;
    try
    {
        m_audio.openStream(
            &parameters,
            nullptr,
            RTAUDIO_SINT16,
            decoder.frequency(),
            &frames,
            [](
                void* outputBuffer,
                void* inputBuffer,
                unsigned int nFrames,
                double streamTime,
                RtAudioStreamStatus status,
                void* userData
            )
            {
                int16_t* out = static_cast<int16_t*>(outputBuffer);
                AudioDecoder* decoder = (AudioDecoder*)userData;
                if (status) Logger::error("Audio stream underflow detected!");
                decoder->decode(out, nFrames * decoder->channels());
                return 0;
            },
            &decoder
        );
        m_audio.startStream();
    }
    catch (RtAudioError& e)
    {
        Logger::error("[Rtaudio]", e.getMessage());
        throw;
    }
}

void SoundPlayer::destroy()
{
    try
    {
        // Stop the stream
        m_audio.stopStream();
    }
    catch (RtAudioError& e)
    {
        Logger::error("[Rtaudio]", e.getMessage());
        throw;
    }
    if (m_audio.isStreamOpen())
        m_audio.closeStream();
}

};