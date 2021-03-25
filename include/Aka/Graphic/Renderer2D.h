#pragma once

#include <Aka/Core/Geometry.h>
#include <Aka/Core/Font.h>
#include <Aka/Graphic/Batch2D.h>
#include <Aka/Graphic/Shader.h>
#include <Aka/Graphic/Framebuffer.h>
#include <Aka/Graphic/Texture.h>
#include <Aka/Graphic/Mesh.h>
#include <Aka/Graphic/RenderPass.h>
#include <Aka/Graphic/GraphicBackend.h>

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

	// Draw a rect in batch
	static void draw(const mat3f& transform, Batch2D::Rect&& rect);
	static void draw(const mat3f& transform, Batch2D::Text&& text);
	static void draw(const mat3f& transform, Batch2D::Line&& line);
	static void draw(const mat3f& transform, Batch2D::Quad&& quad);
	static void draw(const mat3f& transform, Batch2D::Tri&& tri);

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