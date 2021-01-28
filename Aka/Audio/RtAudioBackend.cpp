#include "AudioBackend.h"

#include <RtAudio.h>

#include "../OS/Logger.h"
#include "../Core/Debug.h"

namespace aka {

struct RtAudioContext {
    uint32_t frequency;
    uint32_t channels;
    RtAudio *audio;
};

RtAudioContext ctx;

void AudioBackend::initialize(uint32_t frequency, uint32_t channels)
{
    ctx.channels = channels;
    ctx.frequency = frequency;
    ctx.audio = new RtAudio(RtAudio::Api::WINDOWS_DS);
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
    uint32_t frames = 4096;
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
                int16_t* out = static_cast<int16_t*>(outputBuffer);
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
        ctx.audio->startStream();
    }
    catch (RtAudioError& e)
    {
        Logger::error("[Rtaudio]", e.getMessage());
        throw;
    }
}

void AudioBackend::destroy()
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
    if (ctx.audio->isStreamOpen())
        ctx.audio->closeStream();
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

};