#pragma once

namespace aka {

using AudioFrame = int16_t;

struct AudioSampleStereo
{
    AudioFrame left, right;
};

// Raw audio
struct Audio {
    Vector<AudioFrame> frames;
    float duration;
    uint32_t channels;
    uint32_t frequency;
};

};