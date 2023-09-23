#pragma once 

#include <Aka/Resource/Archive/ArchiveStaticMesh.hpp>
#include <Aka/Resource/Resource/Texture.hpp>

#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

class StaticMesh : public Resource {
public:
	StaticMesh();
	StaticMesh(ResourceID _id, const String& _name);

private:
	void create_internal(AssetLibrary* _library, Renderer* _renderer, const Archive& _archive) override;
	void save_internal(AssetLibrary* _library, Renderer* _renderer, Archive& _archive) override;
	void destroy_internal(AssetLibrary* _library, Renderer* _renderer) override;

public:
	gfx::IndexFormat getIndexFormat() const { return m_indexFormat; }
	aabbox<> getBounds() const { return m_bounds; }
public: // Optionnal data for runtime operations
	gfx::VertexAttributeState attributes;
public: // Mandatory data for rendering & co
	// Should be mutualized in a big single geometry buffer with pages.
	gfx::BufferHandle gfxVertexBuffer;
	gfx::BufferHandle gfxIndexBuffer;
	gfx::SamplerHandle gfxAlbedoSampler;
	gfx::SamplerHandle gfxNormalSampler;
	gfx::IndexFormat m_indexFormat;
	aabbox<> m_bounds;

	struct DrawCallIndexed {
		uint32_t vertexOffset;
		uint32_t indexOffset;
		uint32_t indexCount;
		// TODO mutualize texture to avoid copies
		ResourceHandle<Texture> m_albedo;
		ResourceHandle<Texture> m_normal;
		gfx::BufferHandle gfxUniformBuffer;
		gfx::DescriptorSetHandle gfxDescriptorSet;
	};
	Vector<DrawCallIndexed> batches; // TODO indirect buffer. Require bindless for material
};


}