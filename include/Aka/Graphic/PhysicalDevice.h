#pragma once

#include <stdint.h>

namespace aka {
namespace gfx {

enum class PhysicalDeviceFeatures
{
	None = 0,

	BindlessResources = 1 << 0,
	MeshShader = 1 << 1,
	RenderDocAttachment = 1 << 2,

	Required = BindlessResources,
	Optional = MeshShader | RenderDocAttachment,
	Default = Required | RenderDocAttachment,

	All = MeshShader | BindlessResources | RenderDocAttachment,
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