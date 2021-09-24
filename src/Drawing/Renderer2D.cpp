#include <Aka/Drawing/Renderer2D.h>

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

void Renderer2D::draw(const mat3f& transform, const Line& line)
{
	batch.draw(transform, line);
}

void Renderer2D::draw(const mat3f& transform, const Quad& quad)
{
	batch.draw(transform, quad);
}

void Renderer2D::draw(const mat3f& transform, const Poly& poly)
{
	batch.draw(transform, poly);
}

void Renderer2D::draw(const mat3f& transform, const Triangle& tri)
{
	batch.draw(transform, tri);
}

void Renderer2D::drawLine(const mat3f& transform, const vec2f& A, const vec2f& B, const color4f& color, int32_t layer)
{
	Line line;
	line.vertices[0] = Vertex{ A, uv2f(0.f), color };
	line.vertices[1] = Vertex{ B, uv2f(1.f), color };
	line.layer = layer;
	Renderer2D::draw(transform, line);
}

void Renderer2D::drawCircle(const mat3f& transform, const vec2f& position, float radius, const color4f& color, int32_t layer)
{
	Poly poly;
	poly.vertices.push_back(Vertex{
		position,
		uv2f(0.f),
		color
	});
	const uint32_t resolution = 16;
	for (uint32_t i = 0; i <= resolution; i++)
	{
		anglef angle = i * 2.f * pi<float> / resolution;
		poly.vertices.push_back(Vertex{
			vec2f(radius * cos(angle) + position.x, radius * sin(angle) + position.y),
			uv2f(cos(angle), sin(angle)),
			color
		});
	}
	poly.layer = layer;
	poly.texture = nullptr;
	poly.primitive = PrimitiveType::TriangleFan;
	Renderer2D::draw(transform, poly);
}

void Renderer2D::drawRect(const mat3f& transform, const vec2f& pos, const vec2f& size, Texture::Ptr texture, const color4f& color, int32_t layer)
{
	Quad quad;
	quad.vertices[0] = Vertex{ vec2f(pos), uv2f(0.f), color };
	quad.vertices[1] = Vertex{ vec2f(pos.x + size.x, pos.y), uv2f(1.f, 0.f), color };
	quad.vertices[2] = Vertex{ vec2f(pos.x, pos.y + size.y), uv2f(0.f, 1.f), color };
	quad.vertices[3] = Vertex{ vec2f(pos + size), uv2f(1.f), color };
	quad.texture = texture;
	quad.layer = layer;
	Renderer2D::draw(transform, quad);
}

void Renderer2D::drawRect(const mat3f& transform, const vec2f& pos, const vec2f& size, const uv2f& uv0, const uv2f& uv1, Texture::Ptr texture, const color4f& color, int32_t layer)
{
	Quad quad;
	quad.vertices[0] = Vertex{ vec2f(pos), uv2f(uv0), color };
	quad.vertices[1] = Vertex{ vec2f(pos.x + size.x, pos.y), uv2f(uv1.u, uv0.v), color };
	quad.vertices[2] = Vertex{ vec2f(pos.x, pos.y + size.y), uv2f(uv0.u, uv1.v), color };
	quad.vertices[3] = Vertex{ vec2f(pos + size), uv2f(uv1), color };
	quad.texture = texture;
	quad.layer = layer;
	Renderer2D::draw(transform, quad);
}

void Renderer2D::drawText(const mat3f& transform, const String& text, const Font& font, const color4f& color, int32_t layer)
{
	float scale = 1.f;
	float advance = 0.f;
	const char* start = text.begin();
	const char* end = text.end();
	while (start < end)
	{
		uint32_t c = encoding::next(start, end);
		// TODO check if rendering text out of screen for culling ?
		const Character& ch = font.getCharacter(c);
		vec2f position = vec2f(advance + ch.bearing.x, (float)-(ch.size.y - ch.bearing.y)) * scale;
		vec2f size = vec2f((float)ch.size.x, (float)ch.size.y) * scale;
		Renderer2D::drawRect(transform, position, size, ch.texture.get(0), ch.texture.get(1), ch.texture.texture, color, layer);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		advance += ch.advance * scale;
	}
}

void Renderer2D::render()
{
	render(GraphicBackend::device()->backbuffer());
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