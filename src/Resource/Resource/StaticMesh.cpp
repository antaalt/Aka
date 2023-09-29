#include <Aka/Resource/Resource/StaticMesh.hpp>

#include <Aka/Resource/AssetLibrary.hpp>
#include <Aka/Renderer/Renderer.hpp>

namespace aka {

struct MaterialUniformBuffer {
	color4f color;
};

StaticMesh::StaticMesh() :
	Resource(ResourceType::StaticMesh)
{
}

StaticMesh::StaticMesh(AssetID _id, const String& _name) :
	Resource(ResourceType::StaticMesh, _id, _name)
{
}
StaticMesh::~StaticMesh()
{
	AKA_ASSERT(m_batches.size() == 0, "Batches should have been cleared.");
}

void StaticMesh::create_internal(AssetLibrary* _library, Renderer* _renderer, const Archive& _archive)
{
	AKA_ASSERT(_archive.type() == AssetType::StaticMesh, "Invalid archive");
	const ArchiveStaticMesh& meshArchive = reinterpret_cast<const ArchiveStaticMesh&>(_archive);
	// TODO mutualize sampler in Renderer class (& pass renderer instead of device as argument, or renderer create class)
	m_gfxAlbedoSampler = _renderer->getDevice()->createSampler(
		"Sampler",
		gfx::Filter::Linear, gfx::Filter::Linear,
		gfx::SamplerMipMapMode::Linear,
		gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat,
		1.0
	);
	m_gfxNormalSampler = _renderer->getDevice()->createSampler(
		"Sampler",
		gfx::Filter::Linear, gfx::Filter::Linear,
		gfx::SamplerMipMapMode::Linear,
		gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat,
		1.0
	);
	// TODO should retrieve this from shader somehow...
	gfx::ShaderBindingState bindings{};
	bindings.add(gfx::ShaderBindingType::UniformBuffer, gfx::ShaderMask::Vertex | gfx::ShaderMask::Fragment, 1);
	bindings.add(gfx::ShaderBindingType::SampledImage, gfx::ShaderMask::Fragment, 1);
	bindings.add(gfx::ShaderBindingType::SampledImage, gfx::ShaderMask::Fragment, 1);
	m_pool = _renderer->getDevice()->createDescriptorPool("MeshDescriptorPool", bindings, (uint32_t)meshArchive.batches.size());
	Vector<StaticVertex> vertices;
	Vector<uint32_t> indices;
	for (const ArchiveBatch& batch : meshArchive.batches)
	{
		// Material
		// TODO mips
		ResourceHandle<Texture> albedo = _library->load<Texture>(batch.material.albedo.id(), batch.material.albedo, _renderer);
		ResourceHandle<Texture> normal = _library->load<Texture>(batch.material.normal.id(), batch.material.normal, _renderer);
		
		MaterialUniformBuffer ubo{};
		ubo.color = batch.material.color;
		gfx::BufferHandle gfxUniformBuffer = _renderer->getDevice()->createBuffer("MaterialUniformBuffer", gfx::BufferType::Uniform, sizeof(MaterialUniformBuffer), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, &ubo);
		
		gfx::DescriptorSetHandle gfxDescriptorSet = _renderer->getDevice()->allocateDescriptorSet("DescriptorSetMaterial", bindings, m_pool);

		gfx::DescriptorSetData data;
		data.addUniformBuffer(gfxUniformBuffer);
		data.addSampledTexture2D(albedo.get().getGfxHandle(), m_gfxAlbedoSampler);
		data.addSampledTexture2D(normal.get().getGfxHandle(), m_gfxNormalSampler);
		_renderer->getDevice()->update(gfxDescriptorSet, data);
		
		m_batches.append(StaticMeshBatch{
			(uint32_t)(vertices.size() * sizeof(StaticVertex)),
			(uint32_t)(indices.size() * sizeof(uint32_t)),
			(uint32_t)batch.geometry.indices.size(),
			albedo,
			normal,
			gfxUniformBuffer,
			gfxDescriptorSet,
		});

		vertices.append(batch.geometry.vertices);
		indices.append(batch.geometry.indices);
		m_bounds.include(batch.geometry.bounds);
	}
	m_indexFormat = gfx::IndexFormat::UnsignedInt;
	m_gfxVertexBuffer = _renderer->getDevice()->createBuffer("VertexBuffer", gfx::BufferType::Vertex, (uint32_t)(sizeof(StaticVertex) * vertices.size()), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, vertices.data());
	m_gfxIndexBuffer = _renderer->getDevice()->createBuffer("IndexBuffer", gfx::BufferType::Index, (uint32_t)(sizeof(uint32_t) * indices.size()), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, indices.data());;
}

void StaticMesh::save_internal(AssetLibrary* _library, Renderer* _renderer, Archive& _archive)
{
	AKA_NOT_IMPLEMENTED;
}

void StaticMesh::destroy_internal(AssetLibrary* _library, Renderer* _renderer)
{
	_renderer->getDevice()->destroy(m_gfxAlbedoSampler);
	_renderer->getDevice()->destroy(m_gfxNormalSampler);
	_renderer->getDevice()->destroy(m_gfxIndexBuffer);
	_renderer->getDevice()->destroy(m_gfxVertexBuffer);
	for (const StaticMeshBatch& batch : m_batches)
	{
		_renderer->getDevice()->free(batch.gfxDescriptorSet);
		_renderer->getDevice()->destroy(batch.gfxUniformBuffer);
	}
	_renderer->getDevice()->destroy(m_pool);
	m_batches.clear();
}

}