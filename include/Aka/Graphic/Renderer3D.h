#pragma once

#include <Aka/Core/Geometry.h>
#include <Aka/Graphic/Batch3D.h>
#include <Aka/Graphic/Framebuffer.h>

namespace aka {

class Renderer3D 
{
public:
	static void initialize();
	static void destroy();
	static void frame();

	using Vertex = Batch3D::Vertex;
	using Triangle = Batch3D::Triangle;
	using Quad = Batch3D::Quad;
	using Line = Batch3D::Line;
	using Poly = Batch3D::Poly;

	static void draw(const mat4f& transform, const Triangle& triangle);
	static void draw(const mat4f& transform, const Quad& quad);
	static void draw(const mat4f& transform, const Line& line);
	static void draw(const mat4f& transform, const Poly& poly);
	static void drawAxis(const mat4f& transform);
	static void drawFrustum(const mat4f& projection);
	static void drawTransform(const mat4f& transform);

	static void render(Framebuffer::Ptr framebuffer, const mat4f& view, const mat4f& projection);
	static void clear();
private:
	static Batch3D batch;
};

};