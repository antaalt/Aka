#pragma once

#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Core/Event.h>
#include <Aka/Platform/PlatformBackend.h>

namespace aka {

enum class Synchronisation {
	Vertical,
	Unlimited
};

// Backbuffer aka swapchain
// Handle vsync and everything related to display ?
class Backbuffer :
	public Framebuffer,
	EventListener<BackbufferResizeEvent>
{
public:
	using Ptr = std::shared_ptr<Backbuffer>;

	Backbuffer(uint32_t width, uint32_t height);
	Backbuffer(const Backbuffer&) = delete;
	Backbuffer& operator=(const Backbuffer&) = delete;
	virtual ~Backbuffer();

	// Set synchronisation for backbuffer submit
	virtual void set(Synchronisation sync) = 0;
	// Blit a texture to backbuffer
	virtual void blit(const Texture::Ptr& texture, TextureFilter filter) = 0;
	// Start a new frame for backbuffer
	virtual void frame() = 0;
	// Submit current frame for backbuffer
	virtual void submit() = 0;
	// download current frame content
	virtual void download(void* data) = 0;
private:
	// Resize the backbuffer.
	virtual void resize(uint32_t width, uint32_t height) = 0;
	// Called when window get resized and framebuffer need resize.
	void onReceive(const BackbufferResizeEvent& event) override;
};

};