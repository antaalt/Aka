#include <Aka/Resource/Resource/StaticMesh.hpp>

#include <Aka/Resource/AssetLibrary.hpp>

namespace aka {

struct MaterialUniformBuffer {
	color4f color;
};

StaticMesh::StaticMesh() :
	Resource(ResourceType::StaticMesh)
{
}

StaticMesh::StaticMesh(ResourceID _id, const String& _name) : 
	Resource(ResourceType::StaticMesh, _id, _name)
{
}

void StaticMesh::create_internal(AssetLibrary* _library, gfx::GraphicDevice* _device, const Archive& _archive)
{
	AKA_ASSERT(_archive.type() == AssetType::StaticMesh, "Invalid archive");
	const ArchiveStaticMesh& meshArchive = reinterpret_cast<const ArchiveStaticMesh&>(_archive);
	// TODO mutualize sampler in Renderer class (& pass renderer instead of device as argument, or renderer create class)
	this->gfxAlbedoSampler = _device->createSampler(
		"Sampler",
		gfx::Filter::Linear, gfx::Filter::Linear,
		gfx::SamplerMipMapMode::Linear,
		gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat,
		1.0
	);
	this->gfxNormalSampler = _device->createSampler(
		"Sampler",
		gfx::Filter::Linear, gfx::Filter::Linear,
		gfx::SamplerMipMapMode::Linear,
		gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat, gfx::SamplerAddressMode::Repeat,
		1.0
	);
	Vector<Vertex> vertices;
	Vector<uint32_t> indices;
	for (const ArchiveBatch& batch : meshArchive.batches)
	{
		// TODO should retrieve this from shader somehow...
		gfx::ShaderBindingState bindings{};
		bindings.add(gfx::ShaderBindingType::UniformBuffer, gfx::ShaderMask::Vertex | gfx::ShaderMask::Fragment, 1);
		bindings.add(gfx::ShaderBindingType::SampledImage, gfx::ShaderMask::Fragment, 1);
		bindings.add(gfx::ShaderBindingType::SampledImage, gfx::ShaderMask::Fragment, 1);
		// Material
		// TODO mips
		ResourceHandle<Texture> albedo = _library->load<Texture>(_library->getResourceID(batch.material.albedo.id()), batch.material.albedo, _device);
		ResourceHandle<Texture> normal = _library->load<Texture>(_library->getResourceID(batch.material.normal.id()), batch.material.normal, _device);
		
		MaterialUniformBuffer ubo{};
		ubo.color = batch.material.color;
		gfx::BufferHandle gfxUniformBuffer = _device->createBuffer("MaterialUniformBuffer", gfx::BufferType::Uniform, sizeof(MaterialUniformBuffer), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, &ubo);
		
		gfx::DescriptorSetHandle gfxDescriptorSet = _device->createDescriptorSet("DescriptorSetMaterial", bindings);

		gfx::DescriptorSetData data;
		data.addUniformBuffer(gfxUniformBuffer);
		data.addSampledTexture2D(albedo.get().getGfxHandle(), this->gfxAlbedoSampler);
		data.addSampledTexture2D(normal.get().getGfxHandle(), this->gfxNormalSampler);
		_device->update(gfxDescriptorSet, data);
		
		this->batches.append(DrawCallIndexed{
			(uint32_t)(vertices.size() * sizeof(Vertex)),
			(uint32_t)(indices.size() * sizeof(uint32_t)),
			(uint32_t)batch.geometry.indices.size(),
			albedo,
			normal,
			gfxUniformBuffer,
			gfxDescriptorSet,
		});

		vertices.append(batch.geometry.vertices);
		indices.append(batch.geometry.indices);
		this->m_bounds.include(batch.geometry.bounds);
	}
	this->m_indexFormat = gfx::IndexFormat::UnsignedInt;
	this->attributes = Vertex::getState();
	this->gfxVertexBuffer = _device->createBuffer("VertexBuffer", gfx::BufferType::Vertex, (uint32_t)(sizeof(Vertex) * vertices.size()), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, vertices.data());
	this->gfxIndexBuffer = _device->createBuffer("IndexBuffer", gfx::BufferType::Index, (uint32_t)(sizeof(uint32_t) * indices.size()), gfx::BufferUsage::Default, gfx::BufferCPUAccess::None, indices.data());;
}

void StaticMesh::save_internal(AssetLibrary* _library, gfx::GraphicDevice* _device, Archive& _archive)
{
	AKA_NOT_IMPLEMENTED;
}

void StaticMesh::destroy_internal(AssetLibrary* _library, gfx::GraphicDevice* _device)
{
	_device->destroy(this->gfxAlbedoSampler);
	_device->destroy(this->gfxNormalSampler);
	_device->destroy(this->gfxIndexBuffer);
	_device->destroy(this->gfxVertexBuffer);
	for (const DrawCallIndexed& batch : batches)
	{
		_device->destroy(batch.gfxDescriptorSet);
		_device->destroy(batch.gfxUniformBuffer);
	}
	this->attributes = gfx::VertexAttributeState{};
	this->batches.clear();
}

}