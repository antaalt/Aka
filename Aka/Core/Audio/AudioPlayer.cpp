#include "AudioPlayer.h"

#include "../../Platform/Logger.h"
#include "../../Core/Debug.h"

#include "AudioDecoderMp3.h"

namespace aka {


AudioPlayer::AudioPlayer(uint32_t frequency, uint32_t channels) :
    m_frequency(frequency),
    m_channels(channels),
    m_volume(1.f),
    m_audio(RtAudio::Api::WINDOWS_DS)
{ 
    // Determine the number of devices available
    unsigned int devices = m_audio.getDeviceCount();
    if (devices == 0)
        throw std::runtime_error("No audio devices found");

    RtAudio::StreamParameters parameters;
    parameters.deviceId = m_audio.getDefaultOutputDevice();
    parameters.nChannels = m_channels;
    parameters.firstChannel = 0;

    // Frames is not samples
    // One frame is equivalent to samples for all the channels
    // Sample = channel * frames
    uint32_t frames = 4096;
    try
    {
        m_audio.openStream(
            &parameters,
            nullptr,
            RTAUDIO_SINT16,
            m_frequency,
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
                AudioPlayer* player = (AudioPlayer*)userData;
                if (status) {
                    if (status & RTAUDIO_INPUT_OVERFLOW) {
                        Logger::warn("Audio stream overflow detected!");
                    }
                    if (status & RTAUDIO_OUTPUT_UNDERFLOW) {
                        Logger::warn("Audio stream underflow detected!");
                    }
                }
                player->process(out, nFrames);
                return 0;
            },
            this
        );
        m_audio.startStream();
    }
    catch (RtAudioError& e)
    {
        Logger::error("[Rtaudio]", e.getMessage());
        throw;
    }
}

AudioPlayer::~AudioPlayer()
{
    try
    {
        // Stop the stream
        if(m_audio.isStreamRunning())
            m_audio.stopStream();
    }
    catch (RtAudioError& e)
    {
        Logger::error("[Rtaudio]", e.getMessage());
    }
    if (m_audio.isStreamOpen())
        m_audio.closeStream();
}

uint32_t AudioPlayer::getDeviceCount()
{
    return m_audio.getDeviceCount();
    /*if (devices == 0)
        throw std::runtime_error("No audio devices found");
    // Scan through devices for various capabilities
    RtAudio::DeviceInfo info;
    for (unsigned int i = 0; i < devices; i++) {
        info = audio.getDeviceInfo(i);
        if (info.probed == true) {
            Logger::info("Audio device ", i, " : ", info.name);
        }
    }*/
}

AudioDecoder::ID generateUniqueHandle(AudioDecoder* decoder)
{
    ASSERT(sizeof(void*) == sizeof(uint64_t), "AudioDecoder::ID might not works correctly. Make a better implementation.");
    return AudioDecoder::ID((uintptr_t)decoder);
}

AudioDecoder::ID AudioPlayer::play(const Path& path, bool loop)
{
    std::unique_ptr<AudioDecoder> decoder;
    if (path.extension() == ".mp3")
    {
        decoder = std::make_unique<AudioDecoderMp3>(path, loop);
    }
    else
    {
        Logger::error("Audio format not supported");
        return AudioDecoder::ID(0);
    }
    ASSERT(m_frequency == decoder->frequency(), "Audio will need resampling");
    ASSERT(m_channels == decoder->channels(), "Audio channels does not match");
    AudioDecoder::ID id = generateUniqueHandle(decoder.get());
    std::lock_guard<std::mutex> m(m_lock);
    m_decoders.insert(std::make_pair(id, std::move(decoder)));
    return id;
}

bool AudioPlayer::finished(AudioDecoder::ID id)
{
    auto it = m_decoders.find(id);
    if (it == m_decoders.end())
        return true;
    else
        return !it->second->playing();
}

bool AudioPlayer::exist(AudioDecoder::ID id)
{
    auto it = m_decoders.find(id);
    return (it != m_decoders.end());
}

void AudioPlayer::close(AudioDecoder::ID id)
{
    auto it = m_decoders.find(id);
    std::lock_guard<std::mutex> m(m_lock);
    m_decoders.erase(it);
}

int16_t mix(int16_t sample1, int16_t sample2)
{
    const int32_t result(static_cast<int32_t>(sample1) + static_cast<int32_t>(sample2));
    using range = std::numeric_limits<int16_t>;
    if (range::max() < result)
        return range::max();
    else if (range::min() > result)
        return range::min();
    else
        return result;
}

void AudioPlayer::process(int16_t* buffer, uint32_t frames)
{
    std::lock_guard<std::mutex> m(m_lock);
    if (m_decoders.size() == 0)
        return; // No audio to process
    // Set buffer to zero for mixing
    for (unsigned int i = 0; i < frames * m_channels; i++)
        buffer[i] = 0;
    // Mix all audiodecoder
    std::vector<int16_t> tmp(frames * m_channels);
    for (auto &decoder : m_decoders)
    {
        decoder.second->decode(tmp.data(), frames * m_channels);
        for (unsigned int i = 0; i < frames * m_channels; i++)
            buffer[i] = mix(buffer[i], static_cast<int16_t>(tmp[i] * m_volume));
    }
}

};