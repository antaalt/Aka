#pragma once

#include "Image.h"
#include "Logger.h"
#include "../Core/Debug.h"

namespace aka {

// Basic packer to create font atlas
// TODO optimize
// https://en.wikipedia.org/wiki/Bin_packing_problem
struct Packer {
    Packer(uint32_t elements, uint32_t elementWidth, uint32_t elementHeight);

    // Add an image to the packer
    void add(uint32_t id, const Image &image);
    // Add data to the packer
    void add(uint32_t id, uint32_t width, uint32_t height, const uint8_t* data);

    // Pack the atlas and return the result.
    const Image& pack();

    // Get the final image
    const Image& getImage() const;

    // Get a region on the atlas of the given id
    Rect getRegion(uint32_t id) const;
private:
    bool m_packed; // Is the atlas packed ?
    uint32_t m_elements; // Number of elements in the atlas
    vec2u m_elementCount; // Number of possible elements in the 2D atlas
    vec2u m_elementSize; // Fixed size of each element
    Image m_image; // Final image
    std::vector<Rect> m_regions; // Different regions of the atlas
};

};
