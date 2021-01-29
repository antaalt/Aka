#include "AudioBackend.h"

#include <map>
#include <mutex>
#include <memory>

#include "../OS/Logger.h"
#include "../Core/Debug.h"
#include "AudioDecoder.h"
#include "Codec/AudioDecoderMp3.h"

namespace aka {

struct AudioContext
{
    std::mutex lock;
};

AudioContext ctx;
std::map<AudioID, std::unique_ptr<AudioDecoder>> decoders;


AudioID generateUniqueHandle(AudioDecoder* decoder)
{
    ASSERT(sizeof(void*) == sizeof(uint64_t), "AudioDecoder::ID might not works correctly. Make a better implementation.");
    return AudioID((uintptr_t)decoder);
}

AudioID AudioBackend::play(const Path& path, float volume, bool loop)
{
    std::unique_ptr<AudioDecoder> decoder;
    if (Path::extension(path) == "mp3")
    {
        decoder = std::make_unique<AudioDecoderMp3>(path, volume, loop);
    }
    else
    {
        Logger::error("Audio format not supported : ", Path::extension(path));
        return AudioID(0);
    }
    ASSERT(AudioBackend::getFrequency() == decoder->frequency(), "Audio will need resampling");
    ASSERT(AudioBackend::getChannels() == decoder->channels(), "Audio channels does not match");
    AudioID id = generateUniqueHandle(decoder.get());
    std::lock_guard<std::mutex> m(ctx.lock);
    decoders.insert(std::make_pair(id, std::move(decoder)));
    return id;
}

bool AudioBackend::finished(AudioID id)
{
    auto it = decoders.find(id);
    if (it == decoders.end())
        return true;
    else
        return !it->second->playing();
}

bool AudioBackend::exist(AudioID id)
{
    auto it = decoders.find(id);
    return (it != decoders.end());
}

void AudioBackend::setVolume(AudioID id, float volume)
{
    auto it = decoders.find(id);
    if (it != decoders.end())
    {
        std::lock_guard<std::mutex> m(ctx.lock);
        it->second->volume(volume);
    }
}

void AudioBackend::close(AudioID id)
{
    auto it = decoders.find(id);
    std::lock_guard<std::mutex> m(ctx.lock);
    decoders.erase(it);
}

int16_t mix(int16_t sample1, int16_t sample2)
{
    const int32_t result(static_cast<int32_t>(sample1) + static_cast<int32_t>(sample2));
    using range = std::numeric_limits<int16_t>;
    if ((range::max)() < result)
        return (range::max)();
    else if ((range::min)() > result)
        return (range::min)();
    else
        return result;
}

void AudioBackend::process(int16_t* buffer, uint32_t frames)
{
    std::lock_guard<std::mutex> m(ctx.lock);
    if (decoders.size() == 0)
        return; // No audio to process
    // Set buffer to zero for mixing
    memset(buffer, 0, frames * AudioBackend::getChannels() * sizeof(int16_t));
    // Mix all audiodecoder
    std::vector<int16_t> tmp(frames * AudioBackend::getChannels());
    for (auto& decoder : decoders)
    {
        AudioDecoder* dec = decoder.second.get();
        dec->decode(tmp.data(), frames * AudioBackend::getChannels());
        for (unsigned int i = 0; i < frames * AudioBackend::getChannels(); i++)
            buffer[i] = mix(buffer[i], static_cast<int16_t>(tmp[i] * dec->volume()));
    }
}

};