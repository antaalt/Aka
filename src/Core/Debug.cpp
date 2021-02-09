#include <Aka/Core/Debug.h>

#include <Aka/OS/Logger.h>

namespace aka {

RuntimeError::RuntimeError(const char* str) :
    RuntimeError(std::string(str))
{
}
RuntimeError::RuntimeError(const std::string& str) :
    m_error(str)
{
    Logger::critical(str);
}
const char* RuntimeError::what() const
{
    return nullptr;
}

};