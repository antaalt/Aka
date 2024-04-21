#pragma once

#include <functional>

#include <Aka/OS/Path.h>

namespace aka {

enum class FileWatchAction
{
	Unknown,

	Added,
	Removed,
	Updated,
};

using FileWatchEvent = std::function<void(const Path& directory, const Path& file, FileWatchAction action)>;

class FileWatcher final {
public:
	FileWatcher();
	FileWatcher(const FileWatcher&) = delete;
	FileWatcher& operator=(const FileWatcher&) = delete;
	~FileWatcher();
	
	// Add a watch
	bool addWatch(const Path& folderPath, bool recursive, FileWatchEvent callback);
	void removeWatch(const Path& folderPath);

private:
	struct OSData; // per OS custom data
	OSData* m_osData;
};

};