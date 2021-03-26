#include <Aka/Graphic/Renderer3D.h>

namespace aka {

Renderer3D::Poly createAxisPoly()
{
	Renderer3D::Poly poly;
	poly.primitive = PrimitiveType::Lines;
	poly.vertices.push_back(Renderer3D::Vertex{ point3f(0, 0, 0), norm3f(0, 1, 0), uv2f(0, 1), color4f(0,0,0,1) });
	poly.vertices.push_back(Renderer3D::Vertex{ point3f(1, 0, 0), norm3f(0, 1, 0), uv2f(0, 1), color4f(1,0,0,1) });
	poly.vertices.push_back(Renderer3D::Vertex{ point3f(0, 0, 0), norm3f(0, 1, 0), uv2f(0, 1), color4f(0,0,0,1) });
	poly.vertices.push_back(Renderer3D::Vertex{ point3f(0, 1, 0), norm3f(0, 1, 0), uv2f(0, 1), color4f(0,1,0,1) });
	poly.vertices.push_back(Renderer3D::Vertex{ point3f(0, 0, 0), norm3f(0, 1, 0), uv2f(0, 1), color4f(0,0,0,1) });
	poly.vertices.push_back(Renderer3D::Vertex{ point3f(0, 0, 1), norm3f(0, 1, 0), uv2f(0, 1), color4f(0,0,1,1) });

	return poly;
}

Renderer3D::Poly createCubePoly(PrimitiveType type)
{
	Renderer3D::Poly poly;
	poly.primitive = type;

	switch (type)
	{
	case PrimitiveType::Lines: {
		uv2f uv = uv2f(0.f);
		norm3f normal = norm3f(0.f, 1.f, 0.f);
		color4f color = color4f(1.f);
		uint8_t offset = 0;
		// Face 1
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1, -1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1, -1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1,  1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1,  1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1, -1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1,  1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1, -1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1,  1, -1), normal, uv, color });

		// Face 2
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1, -1,  1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1, -1,  1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1,  1,  1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1,  1,  1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1, -1,  1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1,  1,  1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1, -1,  1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1,  1,  1), normal, uv, color });

		// Face connect
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1, -1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1, -1,  1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1,  1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(-1,  1,  1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1, -1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1, -1,  1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1,  1, -1), normal, uv, color });
		poly.vertices.push_back(Renderer3D::Vertex{ point3f(1,  1,  1), normal, uv, color });
		break;
	}
	case PrimitiveType::Triangles: {
		// FRONT     BACK
		// 2______3__4______5
		// |      |  |      |
		// |      |  |      |
		// |      |  |      |
		// 0______1__6______7
		point3f p[8] = {
			point3f(-1, -1,  1),
			point3f(1, -1,  1),
			point3f(-1,  1,  1),
			point3f(1,  1,  1),
			point3f(-1,  1, -1),
			point3f(1,  1, -1),
			point3f(-1, -1, -1),
			point3f(1, -1, -1)
		};
		norm3f n[6] = {
			norm3f(0.f,  0.f,  1.f),
			norm3f(0.f,  1.f,  0.f),
			norm3f(0.f,  0.f, -1.f),
			norm3f(0.f, -1.f,  0.f),
			norm3f(1.f,  0.f,  0.f),
			norm3f(-1.f,  0.f,  0.f)
		};
		uv2f u[4] = {
			uv2f(0.f, 0.f),
			uv2f(1.f, 0.f),
			uv2f(0.f, 1.f),
			uv2f(1.f, 1.f)
		};
		color4f color = color4f(1.f);
		// Face 1
		poly.vertices.push_back(Renderer3D::Vertex{ p[0], n[0], u[0], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[1], n[0], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[2], n[0], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[2], n[0], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[1], n[0], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[3], n[0], u[3], color });

		// Face 2
		poly.vertices.push_back(Renderer3D::Vertex{ p[2], n[1], u[0], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[3], n[1], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[4], n[1], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[4], n[1], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[3], n[1], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[5], n[1], u[3], color });

		// Face 3
		poly.vertices.push_back(Renderer3D::Vertex{ p[4], n[2], u[0], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[5], n[2], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[6], n[2], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[6], n[2], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[5], n[2], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[7], n[2], u[3], color });

		// Face 4
		poly.vertices.push_back(Renderer3D::Vertex{ p[6], n[3], u[0], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[7], n[3], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[0], n[3], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[0], n[3], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[7], n[3], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[1], n[3], u[3], color });

		// Face 5
		poly.vertices.push_back(Renderer3D::Vertex{ p[1], n[4], u[0], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[7], n[4], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[3], n[4], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[3], n[4], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[7], n[4], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[5], n[4], u[3], color });

		// Face 6
		poly.vertices.push_back(Renderer3D::Vertex{ p[6], n[5], u[0], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[0], n[5], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[4], n[5], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[4], n[5], u[2], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[0], n[5], u[1], color });
		poly.vertices.push_back(Renderer3D::Vertex{ p[2], n[5], u[3], color });
		break;
	}
	}
	return poly;
}

Batch3D Renderer3D::batch;

void Renderer3D::initialize()
{
	batch.initialize();
}

void Renderer3D::destroy()
{
	batch.destroy();
}

void Renderer3D::frame()
{
	batch.clear();
}

void Renderer3D::draw(const mat4f& transform, const Triangle& triangle)
{
	batch.draw(transform, triangle);
}

void Renderer3D::draw(const mat4f& transform, const Quad& quad)
{
	batch.draw(transform, quad);
}

void Renderer3D::draw(const mat4f& transform, const Line& line)
{
	batch.draw(transform, line);
}

void Renderer3D::draw(const mat4f& transform, const Poly& poly)
{
	batch.draw(transform, poly);
}

void Renderer3D::drawAxis(const mat4f& transform)
{
	static Poly poly = createAxisPoly();
	batch.draw(transform, poly);
}

void Renderer3D::drawFrustum(const mat4f& projection)
{
	static Poly poly = createCubePoly(PrimitiveType::Lines);
	batch.draw(mat4f::inverse(projection), poly);
}

void Renderer3D::drawTransform(const mat4f& projection)
{
	static Poly poly = createCubePoly(PrimitiveType::Lines);
	batch.draw(projection, poly);
}

void Renderer3D::render(Framebuffer::Ptr framebuffer, const mat4f& view, const mat4f& projection)
{
	batch.render(framebuffer, view, projection);
}

void Renderer3D::clear()
{
	batch.clear();
}

};