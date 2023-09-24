#pragma once

#include <Aka/Renderer/View.hpp>
#include <Aka/Core/Geometry.h>

namespace aka {

// An instance is generated from couple AssetID + InstanceID
// Bucket by rendering type (static mesh, dynamic mesh...)
// All this will generate buckets.
// Which will get in a good place.

enum class InstanceID : uint32_t {};

struct InstanceData {
	mat4f transform;
	mat4f normal;
};
enum class InstanceType : uint32_t
{
	Unknown,

	StaticMesh3D,
	Sprite2D,
	Text2D,
	Text3D,

	First = StaticMesh3D,
	Last = Text3D,
};

struct Instance 
{
	AssetID assetID;
	ViewTypeMask mask; // An instance is visible in some views only.
	InstanceType type;
	// Transform
	mat4f transform;

	gfx::DescriptorSetHandle descriptorSet; // Temp for instance data, should be moved to vertex buffer
};

};