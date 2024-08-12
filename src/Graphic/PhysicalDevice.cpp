#include <Aka/Graphic/PhysicalDevice.h>

namespace aka {
namespace gfx {


const char* toString(PhysicalDeviceFeatures _features)
{
	switch (_features)
	{
	default:
	case PhysicalDeviceFeatures::None: return "None";
	case PhysicalDeviceFeatures::BindlessResources: return "BindlessResources";
	case PhysicalDeviceFeatures::MeshShader: return "MeshShader";
	case PhysicalDeviceFeatures::Barycentric: return "Barycentric";
	case PhysicalDeviceFeatures::AtomicFloat: return "AtomicFloat";
	case PhysicalDeviceFeatures::RenderDocAttachment: return "RenderDocAttachment";
	}
}

};
};