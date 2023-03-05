#include <Aka/Resource/Resource/StaticMesh.h>

#include <Aka/Resource/Archive/StaticMeshArchive.h>

#include <type_traits>

namespace aka {

StaticMesh::StaticMesh() :
	Resource(ResourceType::Mesh)
{
}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::createBuildData()
{
	if (m_buildData != nullptr)
		return;
	m_buildData = new StaticMeshBuildData;
}

void StaticMesh::createBuildData(gfx::GraphicDevice* device, RenderData* data)
{
	if (m_buildData != nullptr)
		return;
	StaticMeshBuildData* meshBuildData = new StaticMeshBuildData;
	m_buildData = meshBuildData;

	AKA_NOT_IMPLEMENTED;
}

void StaticMesh::destroyBuildData()
{
	if (m_buildData == nullptr)
		return;
	delete m_buildData;
	m_buildData = nullptr;
}

void StaticMesh::createRenderData(gfx::GraphicDevice* device, const BuildData* inBuildData)
{
	if (m_renderData != nullptr)
		return;
	if (inBuildData == nullptr)
		return;

	StaticMeshRenderData* meshRenderData = new StaticMeshRenderData;
	m_renderData = meshRenderData;
	
	// Mesh type
	const StaticMeshBuildData* data = reinterpret_cast<const StaticMeshBuildData*>(inBuildData);

	// Vertices
	meshRenderData->attributeState.count = static_cast<uint32_t>(data->vertexAttributeCount);
	meshRenderData->vertexCount = data->vertexCount;
	meshRenderData->vertexOffset = data->vertexOffset;
	for (uint32_t i = 0; i < data->vertexAttributeCount; i++)
	{
		const StaticMeshBuildData::VertexAttribute& bindings = data->vertexAttributes[i];
		meshRenderData->attributeState.attributes[i] = bindings.attribute;
		meshRenderData->attributeState.offsets[i] = bindings.vertexBufferOffset;
		meshRenderData->vertexAttributes[i].offset = 0; // TODO
		meshRenderData->vertexAttributes[i].count = bindings.getVertexCount(data->vertexBuffers);
		meshRenderData->vertexAttributes[i].binding = bindings.vertexBufferIndex;
	}

	// Create all buffers
	meshRenderData->vertexBufferCount = data->vertexBufferCount;
	for (uint32_t i = 0; i < data->vertexBufferCount; i++)
	{
		meshRenderData->vertexBuffers[i] = device->createBuffer(
			"FileVertexBuffer", // TODO id
			gfx::BufferType::Vertex,
			(uint32_t)data->vertexBuffers[i].size(),
			gfx::BufferUsage::Default,
			gfx::BufferCPUAccess::None,
			data->vertexBuffers[i].data()
		);
	}

	meshRenderData->indexCount = data->getIndexCount();
	meshRenderData->indexFormat = data->indexFormat;
	meshRenderData->indexOffset = 0; // TODO mutualise data with some scene allocator

	meshRenderData->indexBuffer = device->createBuffer(
		"FileIndexBuffer",
		gfx::BufferType::Index,
		(uint32_t)data->indicesBuffer.size(),
		gfx::BufferUsage::Default,
		gfx::BufferCPUAccess::None,
		data->indicesBuffer.data()
	);
}

void StaticMesh::destroyRenderData(gfx::GraphicDevice* device)
{
	if (m_renderData == nullptr)
		return;
	StaticMeshRenderData* meshRenderData = reinterpret_cast<StaticMeshRenderData*>(m_renderData);
	for (uint32_t i = 0; i < meshRenderData->vertexBufferCount; i++)
	{
		device->destroy(meshRenderData->vertexBuffers[i]);
		meshRenderData->vertexBuffers[i] = gfx::BufferHandle::null;
	}
	device->destroy(meshRenderData->indexBuffer);
	meshRenderData->indexBuffer = gfx::BufferHandle::null;
}

ResourceArchive* StaticMesh::createResourceArchive()
{
	return new StaticMeshArchive;
}

StaticMesh* StaticMesh::createInterleaved(const gfx::VertexBindingState& state, const void* vertices, uint32_t vertexCount, gfx::IndexFormat indexFormat, const void* indices, uint32_t indexCount)
{
	StaticMesh* mesh = new StaticMesh; // TODO pool
	mesh->createBuildData();
	StaticMeshBuildData* data = reinterpret_cast<StaticMeshBuildData*>(mesh->getBuildData());
	data->flags = StaticMeshBuildFlag::None;
	data->indexFormat = indexFormat;
	data->indicesBuffer = Blob(indices, indexCount * gfx::VertexBindingState::size(indexFormat));
	data->vertexAttributeCount = state.count;
	for (uint32_t i = 0; i < state.count; i++)
	{
		data->vertexAttributes[i].attribute = state.attributes[i];
		data->vertexAttributes[i].vertexBufferIndex = 0;
		data->vertexAttributes[i].vertexBufferOffset = 0;
		data->vertexAttributes[i].vertexBufferSize = state.stride() * vertexCount;
	}
	data->vertexBufferCount = 1;
	data->vertexBuffers[0] = Blob(vertices, state.stride() * vertexCount);
	data->vertexCount = vertexCount;
	data->vertexOffset = 0;
	return mesh;
}

StaticMesh* StaticMesh::createInterleaved(const gfx::VertexBindingState& state, const void* vertices, uint32_t vertexCount)
{
	StaticMesh* mesh = new StaticMesh; // TODO pool
	mesh->createBuildData();
	StaticMeshBuildData* data = reinterpret_cast<StaticMeshBuildData*>(mesh->getBuildData());
	data->flags = StaticMeshBuildFlag::None;
	data->indexFormat = gfx::IndexFormat::Unknown;
	data->indicesBuffer = Blob();
	data->vertexAttributeCount = state.count;
	for (uint32_t i = 0; i < state.count; i++)
	{
		data->vertexAttributes[i].attribute = state.attributes[i];
		data->vertexAttributes[i].vertexBufferIndex = 0;
		data->vertexAttributes[i].vertexBufferOffset = 0;
		data->vertexAttributes[i].vertexBufferSize = state.stride() * vertexCount;
	}
	data->vertexBufferCount = 1;
	data->vertexBuffers[0] = Blob(vertices, state.stride() * vertexCount);
	data->vertexCount = vertexCount;
	data->vertexOffset = 0;
	return mesh;
}


void StaticMesh::bind(gfx::CommandList* cmd) const
{
	const StaticMeshRenderData* data = reinterpret_cast<const StaticMeshRenderData*>(getRenderData());
	Vector<uint32_t> offset(data->vertexBufferCount, 0U);
	cmd->bindVertexBuffer(data->vertexBuffers, 0, data->vertexBufferCount, offset.data());
	cmd->bindIndexBuffer(data->indexBuffer, data->indexFormat, data->indexOffset);
}

void StaticMesh::bindVertex(gfx::CommandList* cmd) const
{
	const StaticMeshRenderData* data = reinterpret_cast<const StaticMeshRenderData*>(getRenderData());
	Vector<uint32_t> offset(data->vertexBufferCount, 0);
	cmd->bindVertexBuffer(data->vertexBuffers, 0, data->vertexBufferCount, offset.data());
}

void StaticMesh::bindIndex(gfx::CommandList* cmd) const
{
	const StaticMeshRenderData* data = reinterpret_cast<const StaticMeshRenderData*>(getRenderData());
	cmd->bindIndexBuffer(data->indexBuffer, data->indexFormat, data->indexOffset);
}

void StaticMesh::draw(gfx::CommandList* cmd, uint32_t instanceCount) const
{
	const StaticMeshRenderData* data = reinterpret_cast<const StaticMeshRenderData*>(getRenderData());
	cmd->draw(data->vertexCount, data->vertexOffset, instanceCount);
}

void StaticMesh::drawIndexed(gfx::CommandList* cmd, uint32_t instanceCount) const
{
	const StaticMeshRenderData* data = reinterpret_cast<const StaticMeshRenderData*>(getRenderData());
	cmd->drawIndexed(data->indexCount, data->indexOffset, data->vertexOffset, instanceCount);
}

void StaticMesh::drawIndirect(gfx::CommandList* cmd, uint32_t instanceCount) const
{
	const StaticMeshRenderData* data = reinterpret_cast<const StaticMeshRenderData*>(getRenderData());
	//cmd->drawIndirect(data->indexCount, data->indexOffset, data->vertexOffset, instanceCount);
}

void StaticMesh::drawIndexedIndirect(gfx::CommandList* cmd, uint32_t instanceCount) const
{
	const StaticMeshRenderData* data = reinterpret_cast<const StaticMeshRenderData*>(getRenderData());
	//cmd->drawIndexedIndirect(data->indexCount, data->indexOffset, data->vertexOffset, instanceCount);
}

};