#pragma once

#include <stdint.h>

namespace aka {
namespace gfx {

enum class PhysicalDeviceFeatures
{
	None = 0,

	BindlessResources = 1 << 0,
	MeshShader = 1 << 1,
	Barycentric = 1 << 2,
	AtomicFloat = 1 << 3,
	RenderDocAttachment = 1 << 4,

	Required = BindlessResources | Barycentric,
	Optional = MeshShader | RenderDocAttachment | AtomicFloat,

	All = Optional | Required,
	Default = Required | RenderDocAttachment,
};
AKA_IMPLEMENT_BITMASK_OPERATOR(PhysicalDeviceFeatures);

static_assert((PhysicalDeviceFeatures::Optional & PhysicalDeviceFeatures::Required) == PhysicalDeviceFeatures::None);

struct PhysicalDeviceLimits
{
	// Textures
	uint32_t maxTexture1DSize;
	uint32_t maxTexture2DSize;
	uint32_t maxTexture3DSize;
	uint32_t maxTextureCubeSize;
	uint32_t maxTextureLayers;
	// Push constants
	uint32_t maxPushConstantSize;
	// Compute
	uint32_t maxComputeWorkgroupSize[3];
	uint32_t maxComputeWorkgroupCount[3];
	uint32_t maxComputeWorkGroupInvocations;
	// Mesh shader
	uint32_t maxMeshShaderWorkgroupSize[3];
	uint32_t maxMeshShaderWorkgroupCount[3];
	uint32_t maxMeshShaderWorkGroupInvocations;
};

};
};