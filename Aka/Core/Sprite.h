#pragma once

#include "../Graphic/Texture.h"
#include "../Platform/Time.h"
#include "Geometry.h"

#include <vector>
#include <string>

namespace aka {

struct Sprite {
	struct Frame {
		// TODO use subtexture with atlas
		Texture::Ptr texture = nullptr;
		Time::Unit duration = Time::Unit();

		void bind() { texture->bind(); }

		static Frame create(Texture::Ptr texture, Time::Unit duration) { Frame frame; frame.texture = texture; frame.duration = duration; return frame; }
	};
	struct Animation {
		std::string name;
		std::vector<Frame> frames;

		// Whole duration of animation
		Time::Unit duration() const {
			Time::Unit duration = Time::Unit();
			for (const Frame& frame : frames)
				duration += frame.duration;
			return duration;
		}
	};

	std::vector<Animation> animations;

	Animation* getAnimation(const std::string& str) {
		for (Animation &animation : animations)
			if (animation.name == str)
				return &animation;
		return nullptr;
	}
};

};

