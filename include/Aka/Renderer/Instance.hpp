#pragma once

#include <Aka/Renderer/View.hpp>
#include <Aka/Core/Geometry.h>
#include <Aka/Resource/Asset.hpp>
#include <Aka/Graphic/GraphicDevice.h>

namespace aka {

// An instance is generated from couple AssetID + InstanceID
// Bucket by rendering type (static mesh, dynamic mesh...)
// All this will generate buckets.
// Which will get in a good place.

enum class InstanceHandle : uint64_t { Invalid = (uint64_t)-1 };

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
	Instance(AssetID assetID, ViewTypeMask mask, InstanceType type) : 
		m_assetID(assetID),
		m_mask(mask),
		m_type(type),
		m_transform(mat4f::identity())
	{
	}
	AssetID getAssetID() const { return m_assetID; }
	ViewTypeMask getViewMask() const { return m_mask; }
	InstanceType getInstanceType() const { return m_type; }
	mat4f getTransform() const { return m_transform; }
	void setTransform(const mat4f& transform) { m_transform = transform; }
private:
	AssetID m_assetID;
	ViewTypeMask m_mask; // An instance is visible in some views only.
	InstanceType m_type;
	mat4f m_transform;
};

};