#include <Aka/Graphic/Resource.h>

#include <Aka/Core/Container/String.h>

namespace aka {
namespace gfx {


Resource::Resource(const char* name, ResourceType type) : 
	name(""), native(ResourceNativeHandleInvalid), 
	type(type) 
{
	String::copy(this->name, sizeof(this->name), name);
	this->name[sizeof(this->name) - 1] = '\0';
}

};
};