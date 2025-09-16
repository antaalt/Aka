#include <Aka/Core/Config.h>

#include <Aka/OS/OS.h>
#include <stdlib.h>
#include <stdarg.h>
#include <cassert>

#if !defined(AKA_PLATFORM_WINDOWS)
// This is a windows specific helper.
int _vscprintf(const char* format, va_list pargs) {
	int retval;
	va_list argcopy;
	va_copy(argcopy, pargs);
	retval = vsnprintf(NULL, 0, format, argcopy);
	va_end(argcopy);
	return retval;
}
#endif

bool handleAssertVariadicList(const char* _filename, int _line, const char* _assertion, bool _allowRecover, const char* message, va_list args)
{
	using namespace aka;
	int length = _vscprintf(message, args);
	assert(length >= 0);

	size_t size = length + 1;
	String messageFormatted = String(size);

	// Format the string
	int ret = vsnprintf(messageFormatted.cstr(), size, message, args);
	assert(ret >= 0);
	String titleFormatted = String::format("Assertion failed in file %s", _filename);
	String messageFormattedFull = String::format("%s\n\nFailed Assertion at %s:%d:\n%s", messageFormatted.cstr(), _filename, _line, _assertion ? _assertion : "error");
	Logger::error(messageFormattedFull);
	AlertModalMessage result = AlertModal(_allowRecover ? AlertModalType::Error : AlertModalType::Critical, titleFormatted.cstr(), messageFormattedFull.cstr());
	switch (result)
	{
	default:
	case AlertModalMessage::Ok:
	case AlertModalMessage::Abort:
		std::abort();
		return false;
	case AlertModalMessage::Retry:
		AKA_DEBUG_BREAK;
		return true; // Do not break
	case AlertModalMessage::Ignore:
		return false; // Do not break
	}
}
bool handleAssert(const char* _filename, int _line, const char* _assertion, bool _allowRecover, const char* message, ...)
{
	va_list args;
	va_start(args, message);
	bool result = handleAssertVariadicList(_filename, _line, _assertion, _allowRecover, message, args);
	va_end(args);
	return result;
}


namespace aka {

};