#include <Aka/Platform/PlatformView.hpp>

namespace aka {

PlatformView::PlatformView(CameraProjection* projection) :
	m_controller(mem::akaNew<VirtualCameraController>(AllocatorMemoryType::Object, AllocatorCategory::Graphic)),
	m_projection(projection)
{
}
PlatformView::PlatformView(CameraController* controller, CameraProjection* projection) :
	m_controller(controller),
	m_projection(projection)
{

}
PlatformView::~PlatformView()
{
	mem::akaDelete(m_controller);
	mem::akaDelete(m_projection);
}

bool PlatformView::update(Time deltaTime)
{
	return m_controller->update(deltaTime);
}

CameraController* PlatformView::getController() const
{
	return m_controller;
}
CameraProjection* PlatformView::getProjection() const
{
	return m_projection;
}

}