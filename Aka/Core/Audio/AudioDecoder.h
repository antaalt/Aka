#pragma once

#include "../../Platform/IO/FileSystem.h"
#include "../StrictType.h"

namespace aka {

class AudioDecoder
{
public:
    using ID = StrictType<uint64_t, struct AudioDecoderTagName>;
public:
    AudioDecoder(bool loop) : m_playing(false), m_loop(loop) {}
    virtual ~AudioDecoder() {}

    virtual bool playing() { return m_playing; }
    virtual void decode(int16_t* buffer, uint32_t bytes) {}
    virtual void seek(uint64_t position) {}
    virtual uint32_t channels() const = 0;
    virtual uint32_t frequency() const = 0;
    virtual uint64_t samples() const = 0;
    virtual size_t fileSize() const = 0;
protected:
    bool m_playing;
    bool m_loop;
};

};