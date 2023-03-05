#include <Aka/Graphic/Resource.h>

#include <Aka/Core/Container/String.h>

namespace aka {
namespace gfx {


Resource::Resource(const char* name, ResourceType type) : 
	name(""), native(ResourceNativeHandleInvalid), 
	type(type) 
{
	String::copy(this->name, 256, name); 
	this->name[255] = '\0'; 
}

};
};