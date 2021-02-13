#include <Aka/Audio/AudioBackend.h>

#include <map>
#include <mutex>
#include <memory>
#include <cstring>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Debug.h>
#include <Aka/Audio/AudioStream.h>
#include <Aka/Audio/AudioStreamMp3.h>
#include <Aka/Audio/AudioStreamMemory.h>

namespace aka {

struct AudioContext
{
    std::mutex lock;
};

struct AudioData {
    float volume;
    bool loop;
};

AudioContext actx;
std::map<AudioStream*, AudioData> audios;

bool AudioBackend::play(AudioStream::Ptr stream, float volume, bool loop)
{
    ASSERT(AudioBackend::getFrequency() == stream->frequency(), "Audio will need resampling");
    ASSERT(AudioBackend::getChannels() == stream->channels(), "Audio channels does not match");
    AudioData audio;
    audio.loop = loop;
    audio.volume = volume;
    std::lock_guard<std::mutex> m(actx.lock);
    auto it = audios.insert(std::make_pair(stream.get(), std::move(audio)));
    if (!it.second)
    {
        it.first->second.loop = loop;
        it.first->second.volume = volume;
        return false;
    }
    return true;
}

void AudioBackend::close(AudioStream::Ptr stream)
{
    auto it = audios.find(stream.get());
    if (it != audios.end())
    {
        std::lock_guard<std::mutex> m(actx.lock);
        audios.erase(it);
    }
}

AudioFrame mix(AudioFrame sample1, AudioFrame sample2)
{
    const int32_t result(static_cast<int32_t>(sample1) + static_cast<int32_t>(sample2));
    using range = std::numeric_limits<AudioFrame>;
    if ((range::max)() < result)
        return (range::max)();
    else if ((range::min)() > result)
        return (range::min)();
    else
        return result;
}

void AudioBackend::process(AudioFrame* buffer, uint32_t frames)
{
    std::lock_guard<std::mutex> m(actx.lock);
    // Set buffer to zero for mixing
    memset(buffer, 0, frames * AudioBackend::getChannels() * sizeof(AudioFrame));
    if (audios.size() == 0)
        return; // No audio to process
    // Mix all audiodecoder
    std::vector<AudioFrame> tmp(frames * AudioBackend::getChannels());
    for (auto& audio : audios)
    {
        AudioStream* stream = audio.first;
        if (!stream->decode(tmp.data(), frames * AudioBackend::getChannels()))
            if (audio.second.loop)
                stream->seek(0);
        for (unsigned int i = 0; i < frames * AudioBackend::getChannels(); i++)
            buffer[i] = mix(buffer[i], static_cast<AudioFrame>(tmp[i] * audio.second.volume));
    }
}

};