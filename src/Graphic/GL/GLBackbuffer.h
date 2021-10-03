#if defined(AKA_USE_OPENGL)
#pragma once

#include "GLContext.h"

#include <Aka/Graphic/Backbuffer.h>
#include <Aka/Core/Event.h>
#include <Aka/Platform/PlatformDevice.h>

namespace aka {

class GLBackbuffer : public Backbuffer
{
public:
	GLBackbuffer(GLFWwindow* window, uint32_t width, uint32_t height);
	~GLBackbuffer();
public:
	// Clear the backbuffer
	void clear(const color4f& color, float depth, int stencil, ClearMask mask) override;

	// Set synchronisation for backbuffer submit
	void set(Synchronisation sync) override;
	// Blit a texture to backbuffer
	void blit(const Texture::Ptr& texture, TextureFilter filter) override;
	// Start a new frame for backbuffer
	void frame() override;
	// Submit current frame for backbuffer
	void submit() override;
	// download current frame content
	void download(void* data) override;
protected: // Can't set backbuffer attachment
	void resize(uint32_t width, uint32_t height) override;
	void set(AttachmentType type, Texture::Ptr texture, AttachmentFlag flag, uint32_t layer, uint32_t level) override;
private:
	GLFWwindow* m_window;
	bool m_vsync;
};

};

#endif