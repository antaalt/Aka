#include <Aka/Resource/Resource/StaticMesh.hpp>

#include <Aka/Resource/AssetLibrary.hpp>
#include <Aka/Renderer/Renderer.hpp>

namespace aka {

gfx::VertexBufferLayout StaticVertex::getState()
{
	gfx::VertexBufferLayout attributes{};
	attributes.add(gfx::VertexSemantic::Position, gfx::VertexFormat::Float, gfx::VertexType::Vec3);
	attributes.add(gfx::VertexSemantic::Normal, gfx::VertexFormat::Float, gfx::VertexType::Vec3);
	attributes.add(gfx::VertexSemantic::TexCoord0, gfx::VertexFormat::Float, gfx::VertexType::Vec2);
	attributes.add(gfx::VertexSemantic::Color0, gfx::VertexFormat::Float, gfx::VertexType::Vec4);
	return attributes;
}

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

void StaticMesh::fromArchive_internal(ArchiveLoadContext& _context, Renderer* _renderer)
{
	const ArchiveStaticMesh& meshArchive = _context.getArchive<ArchiveStaticMesh>(getID());
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
	for (AssetID batchID : meshArchive.batches)
	{
		const ArchiveBatch& batch = _context.getArchive<ArchiveBatch>(batchID);
		const ArchiveMaterial& materialArchive = _context.getArchive<ArchiveMaterial>(batch.material);
		const ArchiveGeometry& geometryArchive = _context.getArchive<ArchiveGeometry>(batch.geometry);
		// Material
		// TODO mips
		ResourceHandle<Texture> albedo = _context.getAssetLibrary()->load<Texture>(materialArchive.albedo, _context, _renderer);
		ResourceHandle<Texture> normal = _context.getAssetLibrary()->load<Texture>(materialArchive.normal, _context, _renderer);
		
		MaterialUniformBuffer ubo{};
		ubo.color = materialArchive.color;
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
			(uint32_t)geometryArchive.indices.size(),
			albedo,
			normal,
			gfxUniformBuffer,
			gfxDescriptorSet,
		});
		vertices.reserve(vertices.size() + geometryArchive.vertices.size());
		for (const ArchiveStaticVertex& archiveVertex : geometryArchive.vertices)
		{
			StaticVertex vertex;
			vertex.position = point3f(archiveVertex.position[0], archiveVertex.position[1], archiveVertex.position[2]);
			vertex.normal = norm3f(archiveVertex.normal[0], archiveVertex.normal[1], archiveVertex.normal[2]);
			vertex.uv = uv2f(archiveVertex.uv[0], archiveVertex.uv[1]);
			vertex.color = color4f(archiveVertex.color[0], archiveVertex.color[1], archiveVertex.color[2], archiveVertex.color[3]);
			vertices.append(vertex);
		}
		indices.append(geometryArchive.indices);
		m_bounds.include(geometryArchive.bounds);
	}
	m_indexFormat = gfx::IndexFormat::UnsignedInt;
	m_gfxVertexBuffer = _renderer->getDevice()->createBuffer("VertexBuffer", gfx::BufferType::Vertex, (uint32_t)(sizeof(StaticVertex) * vertices.size()), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, vertices.data());
	m_gfxIndexBuffer = _renderer->getDevice()->createBuffer("IndexBuffer", gfx::BufferType::Index, (uint32_t)(sizeof(uint32_t) * indices.size()), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, indices.data());;
}

void StaticMesh::toArchive_internal(ArchiveSaveContext& _context, Renderer* _renderer)
{
	AKA_NOT_IMPLEMENTED;
	for (StaticMeshBatch batchID : m_batches)
	{
		AssetID batchAssetID = AssetID::Invalid; // somehow get this.
		AssetID geometryAssetID = AssetID::Invalid; // somehow get this.
		AssetID materialAssetID = AssetID::Invalid; // somehow get this.
		AssetID albedoAssetID = AssetID::Invalid; // somehow get this.
		AssetID normalAssetID = AssetID::Invalid; // somehow get this.
		ArchiveBatch batch(batchAssetID);
		batch.geometry = geometryAssetID;
		batch.material = materialAssetID;
		ArchiveMaterial material(materialAssetID);
		material.albedo = albedoAssetID;
		material.normal = normalAssetID;
		ArchiveGeometry geometry(geometryAssetID);
		geometry.bounds;
		geometry.vertices; // Load from buffer
		geometry.indices; // Load from buffer

		_context.addArchive<ArchiveBatch>(batch.id(), batch);
		_context.addArchive<ArchiveMaterial>(material.id(), material);
		_context.addArchive<ArchiveGeometry>(geometry.id(), geometry);

		_context.getAssetLibrary()->save<Texture>(albedoAssetID, _context, _renderer);
		_context.getAssetLibrary()->save<Texture>(normalAssetID, _context, _renderer);
	}
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