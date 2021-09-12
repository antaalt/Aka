#include <Aka/Audio/AudioStream.h>

#include <Aka/Audio/AudioStreamMemory.h>
#include <Aka/Audio/AudioStreamMp3.h>
#include <Aka/OS/Logger.h>

#include <memory>

namespace aka {

AudioStream::Ptr AudioStream::openStream(const Path& path)
{
    if (File::extension(path) == "mp3")
    {
        AudioStreamMp3::Ptr mp3 = std::make_shared<AudioStreamMp3>();
        if (mp3->open(path))
            return mp3;
        Logger::error("Failed to load audio file : ", File::name(path));
        return nullptr;
    }
    else
    {
        Logger::error("Audio format not supported : ", File::extension(path));
        return nullptr;
    }
}

AudioStream::Ptr AudioStream::loadMemory(const Path& path)
{
    if (File::extension(path) == "mp3")
    {
        AudioStreamMp3 mp3;
        Audio audio;
        if (!mp3.load(path, &audio))
            return nullptr;
        return std::make_shared<AudioStreamMemory>(audio.frames, audio.frequency, audio.channels);
    }
    else
    {
        Logger::error("Audio format not supported : ", File::extension(path));
        return nullptr;
    }
}

};