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

Mp3AudioDecoder::Mp3AudioDecoder(const Path& path)
{
    memset(&m_mp3d, 0, sizeof(m_mp3d));
    int sample = mp3dec_ex_open(&m_mp3d, path.c_str(), MP3D_SEEK_TO_SAMPLE);
    if (!m_mp3d.samples)
        Logger::error("Could not load audio");
}

Mp3AudioDecoder::~Mp3AudioDecoder()
{
    mp3dec_ex_close(&m_mp3d);
    memset(&m_mp3d, 0, sizeof(m_mp3d));
}

bool Mp3AudioDecoder::decode(int16_t* buffer, uint32_t bytes)
{
    memset(buffer, 0, bytes);
    size_t readed = mp3dec_ex_read(&m_mp3d, (mp3d_sample_t*)buffer, bytes);
    ASSERT(m_mp3d.last_error == 0, "Error while reading file");
    return readed == bytes; // Reached eof if condition met
}

void Mp3AudioDecoder::seek()
{
   // mp3dec_ex_seek(&_dec.mp3d, progress);
}

uint32_t Mp3AudioDecoder::channels() const
{
    return static_cast<uint32_t>(m_mp3d.info.channels);
}

uint32_t Mp3AudioDecoder::frequency() const
{
    return static_cast<uint32_t>(m_mp3d.info.hz);
}

uint64_t Mp3AudioDecoder::samples() const
{
    return m_mp3d.samples;
}

size_t Mp3AudioDecoder::fileSize() const
{
    return m_mp3d.file.size;
}

SoundPlayer::SoundPlayer(const Path& path) :
    m_audio(RtAudio::Api::WINDOWS_DS),
    m_decoder(path)
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
    parameters.nChannels = m_decoder.channels();
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
            m_decoder.frequency(),
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
                Mp3AudioDecoder* decoder = (Mp3AudioDecoder*)userData;
                if (status) Logger::error("Audio stream underflow detected!");
                if (decoder->decode(out, nFrames * decoder->channels()))
                    return 0;
                return 1; // EOF
            },
            &m_decoder
            );
        m_audio.startStream();
    }
    catch (RtAudioError& e)
    {
        Logger::error("[Rtaudio]", e.getMessage());
        throw;
    }
}

SoundPlayer::~SoundPlayer()
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

bool SoundPlayer::playing()
{
    return m_audio.isStreamRunning();
}

const Mp3AudioDecoder& SoundPlayer::decoder() const
{
    return m_decoder;
}

};