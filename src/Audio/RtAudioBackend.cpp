#include <Aka/Audio/AudioBackend.h>

#include <set>
#include <mutex>
#include <memory>
#include <cstring>
#include <RtAudio.h>

#include <Aka/OS/Logger.h>
#include <Aka/Core/Debug.h>

#if defined(AKA_PLATFORM_WINDOWS)
#pragma comment(lib, "dsound.lib")
#endif

namespace aka {

struct AudioContext {
    uint32_t frequency = 0;
    uint32_t channels = 0;
    RtAudio *audio = nullptr;
	std::mutex lock;
	std::set<AudioStream::Ptr> audios;
};

static AudioContext ctx;

void AudioBackend::initialize(uint32_t frequency, uint32_t channels)
{
    ctx.channels = channels;
    ctx.frequency = frequency;
#if defined(AKA_PLATFORM_WINDOWS)
    ctx.audio = new RtAudio(RtAudio::Api::WINDOWS_DS);
#else
    ctx.audio = new RtAudio(RtAudio::Api::LINUX_ALSA);
#endif
    // Determine the number of devices available
    unsigned int devices = ctx.audio->getDeviceCount();
    if (devices == 0)
        throw std::runtime_error("No audio devices found");

    RtAudio::StreamParameters parameters;
    parameters.deviceId = ctx.audio->getDefaultOutputDevice();
    parameters.nChannels = ctx.channels;
    parameters.firstChannel = 0;

    // Frames is not samples
    // One frame is equivalent to samples for all the channels
    // Sample = channel * frames
    uint32_t frames = 2048;
    try
    {
        ctx.audio->openStream(
            &parameters,
            nullptr,
            RTAUDIO_SINT16,
            ctx.frequency,
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
                AudioFrame* out = static_cast<AudioFrame*>(outputBuffer);
                if (status) {
                    if (status & RTAUDIO_INPUT_OVERFLOW) {
                        Logger::warn("Audio stream overflow detected!");
                    }
                    if (status & RTAUDIO_OUTPUT_UNDERFLOW) {
                        Logger::warn("Audio stream underflow detected!");
                    }
                }
                AudioBackend::process(out, nFrames);
                return 0;
            },
            nullptr
        );
    }
    catch (RtAudioError& e)
    {
        Logger::error("[Rtaudio]", e.getMessage());
        throw;
    }
    start();
}

void AudioBackend::destroy()
{
	ctx.audios.clear();
    stop();
    if (ctx.audio->isStreamOpen())
        ctx.audio->closeStream();
    delete ctx.audio;
    ctx.audio = nullptr;
}

void AudioBackend::stop()
{
    try
    {
        // Stop the stream
        if (ctx.audio->isStreamRunning())
            ctx.audio->stopStream();
    }
    catch (RtAudioError& e)
    {
        Logger::error("[Rtaudio]", e.getMessage());
    }
}

void AudioBackend::start()
{
    try
    {
        ctx.audio->startStream();
    }
    catch (RtAudioError& e)
    {
        Logger::error("[Rtaudio]", e.getMessage());
    }
}

uint32_t AudioBackend::getDeviceCount()
{
    return ctx.audio->getDeviceCount();
    /*if (devices == 0)
        throw std::runtime_error("No audio devices found");
    // Scan through devices for various capabilities
    RtAudio::DeviceInfo info;
    for (unsigned int i = 0; i < devices; i++) {
        info = audio->getDeviceInfo(i);
        if (info.probed == true) {
            Logger::info("Audio device ", i, " : ", info.name);
        }
    }*/
}

uint32_t AudioBackend::getFrequency()
{
    return ctx.frequency;
}

uint32_t AudioBackend::getChannels()
{
    return ctx.channels;
}

bool AudioBackend::play(AudioStream::Ptr stream)
{
	ASSERT(AudioBackend::getFrequency() == stream->frequency(), "Audio will need resampling");
	ASSERT(AudioBackend::getChannels() == stream->channels(), "Audio channels does not match");
	std::lock_guard<std::mutex> m(ctx.lock);
	auto it = ctx.audios.insert(stream);
	return it.second;
}

void AudioBackend::close(AudioStream::Ptr stream)
{
	std::lock_guard<std::mutex> m(ctx.lock);
	auto it = ctx.audios.find(stream);
	if (it != ctx.audios.end())
		ctx.audios.erase(it);
}
bool AudioBackend::playing(AudioStream::Ptr stream)
{
	std::lock_guard<std::mutex> m(ctx.lock);
	auto it = ctx.audios.find(stream);
	return (it != ctx.audios.end());
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
	std::lock_guard<std::mutex> m(ctx.lock);
	// Set buffer to zero for mixing
	memset(buffer, 0, frames * AudioBackend::getChannels() * sizeof(AudioFrame));
	if (ctx.audios.size() == 0)
		return; // No audio to process
	// Mix all audiodecoder
	std::vector<AudioFrame> tmp(frames * AudioBackend::getChannels());
	for (auto it = ctx.audios.begin(); it != ctx.audios.end();)
	{
		std::set<AudioStream::Ptr>::iterator current = it++;
		AudioStream* stream = (*current).get();
		if (!stream->decode(tmp.data(), frames * AudioBackend::getChannels()))
			ctx.audios.erase(current);
		for (unsigned int i = 0; i < frames * AudioBackend::getChannels(); i++)
			buffer[i] = mix(buffer[i], static_cast<AudioFrame>(tmp[i] * stream->volume()));
	}
}

};