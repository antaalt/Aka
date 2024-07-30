#include <Aka/Graphic/Framebuffer.h>

#include <Aka/Core/Application.h>

#include <type_traits>

namespace aka {
namespace gfx {

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

Framebuffer::Framebuffer(const char* name, uint32_t width, uint32_t height, RenderPassHandle handle, const Attachment* colors, uint32_t count, const Attachment* depth) :
	Resource(name, ResourceType::Framebuffer),
	renderPass(handle),
	width(width),
	height(height),
	count(count),
	depth(depth ? *depth : Attachment{})
{
	memcpy(this->colors, colors, sizeof(Attachment) * count);
	
}

FramebufferHandle Framebuffer::create(const char* name, RenderPassHandle handle, const Attachment* attachments, uint32_t count, const Attachment* depth)
{
	return Application::app()->graphic()->createFramebuffer(name, handle, attachments, count, depth);
}
void Framebuffer::destroy(FramebufferHandle framebuffer)
{
	return Application::app()->graphic()->destroy(framebuffer);
}

};
};