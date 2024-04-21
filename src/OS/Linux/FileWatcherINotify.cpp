#include <Aka/OS/FileWatcher.hpp>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Application.h>

#if defined(AKA_PLATFORM_LINUX)
#include <sys/inotify.h>

namespace aka {

// This code is mostly taken from https://github.com/SpartanJ/efsw
// Thanks for that !


struct FileWatcher::OSData
{
	OSData()
	{
	}
	~OSData()
	{
	}

	void run()
	{
		
	}
};

FileWatcher::FileWatcher() :
	m_osData(mem::akaNew<OSData>(AllocatorMemoryType::Persistent, AllocatorCategory::Default))
{
	// TODO: implement this https://github.com/SpartanJ/efsw/blob/master/src/efsw/FileWatcherInotify.cpp
}
FileWatcher::~FileWatcher()
{
	mem::akaDelete(m_osData);
}

bool FileWatcher::addWatch(const Path& path, bool recursive, FileWatchEvent callback)
{
	AKA_NOT_IMPLEMENTED;
	return false;
}
void FileWatcher::removeWatch(const Path& path)
{
}

}

#endif