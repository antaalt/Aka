#pragma once

#include <Aka/Core/Geometry.h>
#include <Aka/Core/Container/Vector.h>
#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Resource/Shader/ShaderCompiler.h>

namespace aka {

struct DebugVertex
{
	point3f point;
	color4f color;

	static gfx::VertexBufferLayout getVertexLayout() 
	{
		gfx::VertexBufferLayout layout{};
		layout.add(gfx::VertexSemantic::Position, gfx::VertexFormat::Float, gfx::VertexType::Vec3);
		layout.add(gfx::VertexSemantic::Color0, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
		return layout;
	}
};

struct DebugDrawBatch
{
	uint32_t indexOffset;
	uint32_t indexCount;
	gfx::PrimitiveType primitive; // Should have 1 renderer per primitive.
	// is 2D or 3D (to perform matrix operation in screen space.
};

class DebugDrawList
{
protected:
	DebugDrawList() = default;
	friend class Renderer;
	void create(gfx::GraphicDevice* _device, uint32_t width, uint32_t height);
	void destroy(gfx::GraphicDevice* _device);
	void resize(gfx::GraphicDevice* _device, uint32_t width, uint32_t height);
	void prepare(gfx::FrameHandle frame, gfx::GraphicDevice* _device);
	void render(gfx::GraphicDevice* _device, gfx::FrameHandle frame, const mat4f& view, const mat4f& projection);
	void clear();
public:
	void draw3DFrustum(const mat4f& projection, const color4f& color);
	void draw3DSphere(const mat4f& transform, const color4f& color);
	void draw3DPlane(const mat4f& transform, const color4f& color);
	void draw3DCube(const mat4f& transform, const color4f& color);
	void draw3DLine(const point3f* positions, size_t count, const color4f& color);
private:
	Vector<DebugVertex> m_vertices;
	gfx::BufferHandle m_vertexBuffer[gfx::MaxFrameInFlight];
	uint32_t m_vertexBufferSize[gfx::MaxFrameInFlight];
	gfx::RenderPassHandle m_backbufferRenderPass;
	gfx::BackbufferHandle m_backbuffer;
	gfx::GraphicPipelineHandle m_pipeline;
};

};