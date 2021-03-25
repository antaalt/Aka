#include <Aka/Graphic/Renderer2D.h>

namespace aka {

Batch2D Renderer2D::batch;

void Renderer2D::initialize()
{
	batch.initialize();
}

void Renderer2D::destroy()
{
	batch.destroy();
}

void Renderer2D::frame()
{
	batch.clear();
}

void Renderer2D::draw(const mat3f& transform, Batch2D::Rect&& rect)
{
	batch.draw(transform, std::move(rect));
}

void Renderer2D::draw(const mat3f& transform, Batch2D::Text&& text)
{
	batch.draw(transform, std::move(text));
}

void Renderer2D::draw(const mat3f& transform, Batch2D::Line&& line)
{
	batch.draw(transform, std::move(line));
}

void Renderer2D::draw(const mat3f& transform, Batch2D::Quad&& quad)
{
	batch.draw(transform, std::move(quad));
}

void Renderer2D::draw(const mat3f& transform, Batch2D::Tri&& tri)
{
	batch.draw(transform, std::move(tri));
}

void Renderer2D::render()
{
	render(GraphicBackend::backbuffer());
}

void Renderer2D::render(Framebuffer::Ptr framebuffer)
{
	render(framebuffer, mat4f::identity());
}

void Renderer2D::render(Framebuffer::Ptr framebuffer, const mat4f& view)
{
	render(framebuffer, mat4f::identity(), mat4f::orthographic(0.f, (float)framebuffer->height(), 0.f, (float)framebuffer->width()));
}

void Renderer2D::render(Framebuffer::Ptr framebuffer, const mat4f& view, const mat4f& projection)
{
	batch.render(framebuffer, view, projection);
}

void Renderer2D::clear()
{
	batch.clear();
}

};