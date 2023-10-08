#pragma once

#include <Aka/Graphic/GraphicDevice.h>
#include <Aka/Resource/Shader/ShaderRegistry.h>
#include <Aka/Renderer/Instance.hpp>

namespace aka {

// TODO define vertex here ?

// Should be written in a shared glsl file
namespace gpu {
struct SkeletalMeshInstanceData
{
	mat4f transform;
	mat4f normal;
	uint32_t batchIndex;
};

struct SkeletalMeshBatchData
{
	uint32_t vertexOffset;
	uint32_t indexOffset;
	uint32_t indexCount;
	uint32_t boneOffset;
	uint32_t _pad0;
	uint32_t _pad1;
	uint32_t _pad2;

	uint32_t materialIndex;
	// BBOX
	vec4f min;
	vec4f max;
};

struct SkeletalMeshAssetData
{
	uint32_t batchOffset;
	uint32_t batchCount; // valid batches
};

// TODO should use texture to hold all frame from all animations to support instancing.
// check https://developer.nvidia.com/gpugems/gpugems3/part-i-geometry/chapter-2-animated-crowd-rendering
struct SkeletalMeshBoneData
{
	mat4f offset;
};
}

struct SkeletalMeshInstance : Instance
{
	SkeletalMeshInstance(AssetID assetID, ViewTypeMask mask = ViewTypeMask::All) : Instance(assetID, mask, InstanceType::SkeletalMesh3D) {}

	static gfx::VertexBufferLayout getState();
};

};