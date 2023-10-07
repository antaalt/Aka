#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Renderer/Instance.hpp>

namespace aka {

// TODO define vertex here ?

// Should be written in a shared glsl file
namespace gpu {
struct StaticMeshInstanceData
{
	mat4f transform;
	mat4f normal;
	uint32_t batchIndex;
};

struct StaticMeshBatchData
{
	uint32_t vertexOffset;
	uint32_t indexOffset;
	uint32_t indexCount;

	uint32_t materialIndex;
	// BBOX
	vec4f min;
	vec4f max;
};

struct StaticMeshAssetData
{
	uint32_t batchOffset;
	uint32_t batchCount; // valid batches
};
}

struct StaticMeshInstance : Instance
{
	StaticMeshInstance(AssetID assetID, ViewTypeMask mask = ViewTypeMask::All) : Instance(assetID, mask, InstanceType::StaticMesh3D) {}

	static gfx::VertexBufferLayout getState();
};

};