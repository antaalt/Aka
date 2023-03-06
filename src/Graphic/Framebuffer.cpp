#include <Aka/Graphic/Framebuffer.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

FramebufferState getState(GraphicDevice* device, const Attachment* colors, uint32_t count, const Attachment* depth)
{
	FramebufferState framebuffer{};
	framebuffer.count = count;
	for (uint32_t i = 0; i < count; i++)
	{
		if (colors[i].texture == TextureHandle::null)
		{
			framebuffer.colors[i] = {};
		}
		else
		{
			const Texture* texture = device->get(colors[i].texture);
			framebuffer.colors[i].format = texture->format;
			framebuffer.colors[i].loadOp = colors[i].loadOp;
		}
	}
	if (depth == nullptr)
	{
		framebuffer.depth = {};
	}
	else
	{
		const Texture* texture = device->get(depth->texture);
		framebuffer.depth.format = texture->format;
		framebuffer.depth.loadOp = depth->loadOp;
	}
	return framebuffer;
}
uint32_t getWidth(GraphicDevice* device, const Attachment* colors, uint32_t count, const Attachment* depth)
{
	uint32_t width = ~0U;
	for (uint32_t i = 0; i < count; i++)
	{
		if (colors[i].texture != TextureHandle::null)
		{
			const Texture* texture = device->get(colors[i].texture);
			width = min(texture->width, width);
		}
	}
	if (depth != nullptr)
	{
		const Texture* texture = device->get(depth->texture);
		width = min(texture->width, width);
	}
	return width;
}
uint32_t getHeight(GraphicDevice* device, const Attachment* colors, uint32_t count, const Attachment* depth)
{
	uint32_t height = ~0U;
	for (uint32_t i = 0; i < count; i++)
	{
		if (colors[i].texture != TextureHandle::null)
		{
			const Texture* texture = device->get(colors[i].texture);
			height = min(texture->height, height);
		}
	}
	if (depth != nullptr)
	{
		const Texture* texture = device->get(depth->texture);
		height = min(texture->height, height);
	}
	return height;
}

Framebuffer::Framebuffer(const char* name, uint32_t width, uint32_t height, const FramebufferState& state, const Attachment* colors, const Attachment* depth) :
	Resource(name, ResourceType::Framebuffer),
	width(width),
	height(height),
	framebuffer(state)
{
	memcpy(this->colors, colors, sizeof(Attachment) * state.count);
	if (depth)
		this->depth = *depth;
}

FramebufferHandle Framebuffer::create(const char* name, const Attachment* attachments, uint32_t count, const Attachment* depth)
{
	return Application::app()->graphic()->createFramebuffer(name, attachments, count, depth);
}
void Framebuffer::destroy(FramebufferHandle framebuffer)
{
	return Application::app()->graphic()->destroy(framebuffer);
}

bool operator<(const FramebufferState& lhs, const FramebufferState& rhs)
{
	if (lhs.count < rhs.count) return true;
	else if (lhs.count > rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format < rhs.colors[i].format) return true;
		else if (lhs.colors[i].format > rhs.colors[i].format) return false;
		if (lhs.colors[i].loadOp < rhs.colors[i].loadOp) return true;
		else if (lhs.colors[i].loadOp > rhs.colors[i].loadOp) return false;
	}
	if (lhs.depth.format < rhs.depth.format) return true;
	else if (lhs.depth.format > rhs.depth.format) return false;
	if (lhs.depth.loadOp < rhs.depth.loadOp) return true;
	else if (lhs.depth.loadOp > rhs.depth.loadOp) return false;
	return false; // equal
}

bool operator>(const FramebufferState& lhs, const FramebufferState& rhs)
{
	if (lhs.count > rhs.count) return true;
	else if (lhs.count < rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format > rhs.colors[i].format) return true;
		else if (lhs.colors[i].format < rhs.colors[i].format) return false;
		if (lhs.colors[i].loadOp > rhs.colors[i].loadOp) return true;
		else if (lhs.colors[i].loadOp < rhs.colors[i].loadOp) return false;
	}
	if (lhs.depth.format > rhs.depth.format) return true;
	else if (lhs.depth.format < rhs.depth.format) return false;
	if (lhs.depth.loadOp > rhs.depth.loadOp) return true;
	else if (lhs.depth.loadOp < rhs.depth.loadOp) return false;
	return false; // equal
}

bool operator==(const FramebufferState& lhs, const FramebufferState& rhs)
{
	if (lhs.count != rhs.count) return false;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format != rhs.colors[i].format) return false;
		if (lhs.colors[i].loadOp != rhs.colors[i].loadOp) return false;
	}
	if (lhs.depth.format != rhs.depth.format) return false;
	if (lhs.depth.loadOp != rhs.depth.loadOp) return false;
	return true; // equal
}

bool operator!=(const FramebufferState& lhs, const FramebufferState& rhs)
{
	if (lhs.count != rhs.count) return true;
	for (uint32_t i = 0; i < lhs.count; i++)
	{
		if (lhs.colors[i].format != rhs.colors[i].format) return true;
		if (lhs.colors[i].loadOp != rhs.colors[i].loadOp) return true;
	}
	if (lhs.depth.format != rhs.depth.format) return true;
	if (lhs.depth.loadOp != rhs.depth.loadOp) return true;
	return false; // equal
}

};
};