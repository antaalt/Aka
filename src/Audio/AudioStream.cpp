#include <Aka/Audio/AudioStream.h>

#include <Aka/Audio/AudioStreamMemory.h>
#include <Aka/Audio/AudioStreamMp3.h>
#include <Aka/OS/OS.h>
#include <Aka/OS/Logger.h>

#include <memory>

namespace aka {

AudioStream* AudioStream::openStream(const Path& path)
{
    if (OS::File::extension(path) == "mp3")
    {
        AudioStreamMp3* mp3 = new AudioStreamMp3; // TODO pool
        if (mp3->open(path))
            return mp3;
        Logger::error("Failed to load audio file : ", OS::File::name(path));
        return nullptr;
    }
    else
    {
        Logger::error("Audio format not supported : ", OS::File::extension(path));
        return nullptr;
    }
}

AudioStream* AudioStream::loadMemory(const Path& path)
{
    if (OS::File::extension(path) == "mp3")
    {
        AudioStreamMp3 mp3;
        Audio audio;
        if (!mp3.load(path, &audio))
            return nullptr;
        return new AudioStreamMemory(audio.frames, audio.frequency, audio.channels);
    }
    else
    {
        Logger::error("Audio format not supported : ", OS::File::extension(path));
        return nullptr;
    }
}

void AudioStream::destroy(AudioStream* stream)
{
	delete stream;
}

};