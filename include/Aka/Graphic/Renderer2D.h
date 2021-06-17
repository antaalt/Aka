#pragma once

#include <Aka/Core/Geometry.h>
#include <Aka/Graphic/Batch2D.h>
#include <Aka/Graphic/Framebuffer.h>

namespace aka {

class Renderer2D 
{
public:
	// Initialize the 2D renderer
	static void initialize();
	// Destroy the 2D renderer
	static void destroy();

	// Start a new frame
	static void frame();

	using Vertex = Batch2D::Vertex;
	using Triangle = Batch2D::Triangle;
	using Quad = Batch2D::Quad;
	using Line = Batch2D::Line;
	using Poly = Batch2D::Poly;

	// Draw shapes in batch
	static void draw(const mat3f& transform, const Line& line);
	static void draw(const mat3f& transform, const Quad& quad);
	static void draw(const mat3f& transform, const Poly& poly);
	static void draw(const mat3f& transform, const Triangle& tri);
	static void drawLine(const mat3f& transform, const vec2f& A, const vec2f& B, const color4f& color, int32_t layer);
	static void drawRect(const mat3f& transform, const vec2f& pos, const vec2f& size, Texture::Ptr texture, const color4f& color, int32_t layer);
	static void drawRect(const mat3f& transform, const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, Texture::Ptr texture, const color4f& color, int32_t layer);
	static void drawText(const mat3f& transform, const String& text, const Font& font, const color4f& color, int32_t layer);

	// Render to backbuffer
	static void render();
	// Render to specified framebuffer
	static void render(Framebuffer::Ptr framebuffer);
	// Render to specified framebuffer with transform
	static void render(Framebuffer::Ptr framebuffer, const mat4f& view);
	// Render to specified framebuffer with transform & projection
	static void render(Framebuffer::Ptr framebuffer, const mat4f& view, const mat4f& projection);

	// Clear all batch from stack
	static void clear();
private:
	static Batch2D batch;
};

};