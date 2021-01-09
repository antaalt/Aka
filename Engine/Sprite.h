#pragma once

#include "Texture.h"
#include "Geometry.h"
#include "Time.h"

#include <vector>
#include <string>

namespace app {

struct Sprite {
	struct Frame {
		// TODO use subtexture with atlas
		Texture* texture = nullptr;
		Time::unit duration = 0;

		void bind() { texture->bind(); }

		static Frame create(Texture* texture, Time::unit duration) { Frame frame; frame.texture = texture; frame.duration = duration; return frame; }
	};
	struct Animation {
		std::string name;
		std::vector<Frame> frames;

		const Frame* getFrame(Time::unit elapsed) const {

		}

		// Whole duration of animation
		Time::unit duration() const {
			Time::unit duration = 0;
			for (const Frame& frame : frames)
				duration += frame.duration;
			return duration;
		}
	};

	std::vector<Animation> animations;

	vec2f position;
	vec2f size;
	radianf rotation;

	Animation* getAnimation(const std::string& str) {
		for (Animation &animation : animations)
			if (animation.name == str)
				return &animation;
		return nullptr;
	}
};

};
