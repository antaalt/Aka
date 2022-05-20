#include <Aka/Resource/Archive/NullArchive.h>

namespace aka {

bool NullArchive::load(Stream& stream, BuildData* data)
{
	return true; // Always succeed
}

bool NullArchive::save(Stream& stream, const BuildData* data)
{
	return true; // Always succeed
}

};