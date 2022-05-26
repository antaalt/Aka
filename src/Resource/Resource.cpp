#include <Aka/Resource/Resource.h>

#include <Aka/Core/Application.h>
#include <Aka/Resource/Resource/Texture.h>
#include <Aka/Resource/Resource/StaticMesh.h>
#include <Aka/Resource/Resource/Sprite.h>
#include <Aka/Resource/Resource/Font.h>

namespace aka {

Resource::Resource(ResourceType type) :
	m_type(type),
	m_buildData(nullptr),
	m_renderData(nullptr)
{
}

Resource::~Resource()
{
	//destroyRenderData(Application::app()->graphic());
	delete m_renderData;
	destroyBuildData();
}

Resource* Resource::create(ResourceType type)
{
	// TODO memory management
	switch (type)
	{
	case ResourceType::Texture:
		return new Texture;
	case ResourceType::Buffer:
		break;
	case ResourceType::Sprite:
		return new Sprite;
		break;
	case ResourceType::Audio:
		break;
	case ResourceType::Font:
		return new Font;
	case ResourceType::Mesh:
		return new StaticMesh;
	case ResourceType::Scene:
		break;
	default:
		break;
	}
	return nullptr;
}

void Resource::destroy(Resource* resource)
{
	delete resource;
}

void Resource::destroyBuildData()
{
	if (m_buildData == nullptr)
		return;
	delete m_buildData;
	m_buildData = nullptr;
}

RenderData* Resource::getRenderData()
{
	return m_renderData;
}
BuildData* Resource::getBuildData()
{
	return m_buildData;
}
const RenderData* Resource::getRenderData() const
{
	return m_renderData;
}
const BuildData* Resource::getBuildData() const
{
	return m_buildData;
}
ResourceType Resource::getType() const
{
	return m_type;
}

};