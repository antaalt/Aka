#pragma once

#include <Aka/Graphic/Texture2D.h>
#include <Aka/OS/Time.h>
#include <Aka/OS/Path.h>
#include <Aka/OS/Stream/Stream.h>
#include <Aka/Core/Geometry.h>

#include <vector>
#include <string>

namespace aka {

struct Sprite {
	struct Frame {
		// TODO use subtexture with atlas
		Texture2D::Ptr texture = nullptr;
		Time duration = Time();
		uint32_t width = 0;
		uint32_t height = 0;

		static Frame create(Texture2D::Ptr texture, Time duration)
		{ 
			Frame frame; 
			frame.texture = texture; 
			frame.duration = duration; 
			frame.width = texture->width();
			frame.height = texture->height();
			return frame; 
		}
	};
	struct Animation {
		String name;
		std::vector<Frame> frames;

		// Whole duration of animation
		Time duration() const
		{
			Time duration = Time();
			for (const Frame& frame : frames)
				duration += frame.duration;
			return duration;
		}
	};

	std::vector<Animation> animations;

	Animation* getAnimation(const String& str)
	{
		for (Animation &animation : animations)
			if (animation.name == str)
				return &animation;
		return nullptr;
	}

	const Sprite::Frame& getFrame(uint32_t animation, uint32_t frame) const
	{
		return animations[animation].frames[frame];
	}

	// Parse the sprite from an aseprite
	static Sprite parse(Stream& stream);
};

};

