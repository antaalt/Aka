#include <Aka/Core/Config.h>

#include <Aka/OS/OS.h>
#include <stdlib.h>
#include <stdarg.h>

bool handleAssert(const char* _filename, int _line, const char* _assertion, bool _allowRecover, const char* message, ...)
{
	using namespace aka;
	va_list arg1, arg2;
	va_start(arg1, message);
	va_copy(arg2, arg1);
	int length = snprintf(nullptr, 0, message, arg1);
	va_end(arg1);

	size_t size = length + 1;
	String messageFormatted = String(size);

	// Format the string
	vsnprintf(messageFormatted.cstr(), size, message, arg2);
	va_end(arg2);
	String titleFormatted = String::format("Assertion failed in file %s", _filename);
	String messageFormattedFull = String::format("%s\n\nFailed Assertion at %s:%d:\n%s", messageFormatted.cstr(), _filename, _line, _assertion ? _assertion : "error");
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


namespace aka {

};