#pragma once

#include "../../Platform/IO/FileSystem.h"
#include "AudioDecoder.h"

#include <map>
#include <mutex>
#include <RtAudio.h>

namespace aka {

class AudioPlayer
{
public:
    AudioPlayer(uint32_t frequency, uint32_t channels);
    ~AudioPlayer();

    uint32_t getDeviceCount();

    AudioDecoder::ID play(const Path& path, float volume, bool loop = false);
    bool finished(AudioDecoder::ID id);
    bool exist(AudioDecoder::ID id);
    void setVolume(AudioDecoder::ID id, float volume);
    void close(AudioDecoder::ID id);

private:
    void process(int16_t* buffer, uint32_t frames);
private:
    std::mutex m_lock;
    uint32_t m_frequency;
    uint32_t m_channels; // 2 -> stereo, 6 -> 5.1, 8 -> 7.1
    float m_volume;
    RtAudio m_audio;
    std::map<AudioDecoder::ID, std::unique_ptr<AudioDecoder>> m_decoders;
};

};