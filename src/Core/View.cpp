#include <Aka/Core/View.h>

namespace aka {

ViewID generate()
{
	static uint32_t counter = 0;
	return static_cast<ViewID>(counter++);
}

};