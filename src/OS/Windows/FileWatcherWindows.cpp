#include <Aka/OS/FileWatcher.hpp>
#include <Aka/Platform/PlatformDevice.h>
#include <Aka/Core/Application.h>

#include "WindowsCommon.hpp"

#if defined(AKA_PLATFORM_WINDOWS)

namespace aka {

// This code is mostly taken from https://github.com/SpartanJ/efsw
// Thanks for that !

struct Watcher;

struct WatchStruct
{
	OVERLAPPED mOverlapped;
	Watcher* watcher;
};
struct LastEventInfo {
	String fileName;
	size_t fileSize;
	Timestamp modificationTime;
};
struct Watcher
{
	Watcher() :
		mDirHandle(INVALID_HANDLE_VALUE),
		mBuffer(63 * 1024),
		mNotifyFilter(0),
		mStopNow(false),
		mFileWatcher(nullptr),
		mFileWatchEvent(),
		mDirName(),
		mIsRecursive(false)
	{
	}

	HANDLE mDirHandle;
	Vector<BYTE> mBuffer;
	DWORD mNotifyFilter;
	bool mStopNow;
	FileWatcher* mFileWatcher;
	FileWatchEvent mFileWatchEvent;
	Path mDirName;
	bool mIsRecursive;
	LastEventInfo m_lastEvent;

	void handleAction(Watcher* watch, const String& filename, unsigned long action)
	{
		FileWatchAction fwAction;
		switch (action)
		{
		case FILE_ACTION_RENAMED_NEW_NAME:
		case FILE_ACTION_ADDED:
			fwAction = FileWatchAction::Added;
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
		case FILE_ACTION_REMOVED:
			fwAction = FileWatchAction::Removed;
			break;
		case FILE_ACTION_MODIFIED:
			fwAction = FileWatchAction::Updated;
			break;
		default:
			fwAction = FileWatchAction::Unknown;
			return; // Skip unsupported watch.
		};
		watch->mFileWatchEvent(watch->mDirName, Path(filename), fwAction);
	}
};

// forward decl
bool RefreshWatch(WatchStruct* pWatch);

// Unpacks events and passes them to a user defined callback.
void CALLBACK WatchCallback(DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
	if (NULL == lpOverlapped) 
	{
		return;
	}
	PFILE_NOTIFY_INFORMATION pNotify;
	WatchStruct* tWatch = (WatchStruct*)lpOverlapped;
	Watcher* pWatch = tWatch->watcher;
	size_t offset = 0;

	if (dwNumberOfBytesTransfered == 0)
	{
		if (nullptr != pWatch && !pWatch->mStopNow)
		{
			RefreshWatch(tWatch);
		}
		else
		{
			return;
		}
	}
	do
	{
		bool skip = false;

		pNotify = (PFILE_NOTIFY_INFORMATION)&pWatch->mBuffer[offset];
		offset += pNotify->NextEntryOffset;
		int count = WideCharToMultiByte(CP_UTF8, 0, pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR), NULL, 0, NULL, NULL);
		if (count == 0)
			continue;

		String nfile(count, '\0');
		count = WideCharToMultiByte(CP_UTF8, 0, pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR), &nfile[0], count, NULL, NULL);
		BackwardToForwardSlash(nfile);

		if (FILE_ACTION_MODIFIED == pNotify->Action) {

			// Workaround for duplicated modified action from OS for some reason...
			Timestamp timestamp = OS::File::lastWrite(pWatch->mDirName + nfile);
			size_t size = OS::File::size(pWatch->mDirName + nfile);
			LastEventInfo& lastEvent = pWatch->m_lastEvent;
			if ((lastEvent.modificationTime == timestamp && lastEvent.fileSize == size && lastEvent.fileName == nfile) || size == 0)
			{
				skip = true;
			}
			lastEvent.fileName = nfile;
			lastEvent.fileSize = size;
			lastEvent.modificationTime = timestamp;
		}

		if (!skip) 
		{
			pWatch->handleAction(pWatch, nfile, pNotify->Action);
		}
	} while (pNotify->NextEntryOffset != 0);

	if (!pWatch->mStopNow) {
		RefreshWatch(tWatch);
	}
}
// Refreshes the directory monitoring.
bool RefreshWatch(WatchStruct* pWatch)
{
	return ReadDirectoryChangesW(
		pWatch->watcher->mDirHandle,
		pWatch->watcher->mBuffer.data(),
		(DWORD)pWatch->watcher->mBuffer.size(),
		pWatch->watcher->mIsRecursive,
		pWatch->watcher->mNotifyFilter,
		NULL,
		&pWatch->mOverlapped,
		NULL
	) != 0;
}

// Stops monitoring a directory.
void DestroyWatch(WatchStruct* tWatch)
{
	if (tWatch)
	{
		Watcher* pWatch = tWatch->watcher;
		pWatch->mStopNow = TRUE;
		CancelIoEx(pWatch->mDirHandle, &tWatch->mOverlapped);
		CloseHandle(pWatch->mDirHandle);
		mem::akaDelete(pWatch);
		HeapFree(GetProcessHeap(), 0, tWatch);
	}
}

// Starts monitoring a directory.
WatchStruct* CreateWatch(LPCTSTR szDirectory, bool recursive, DWORD mNotifyFilter, HANDLE mIOCP)
{
	WatchStruct* tWatch;
	tWatch = static_cast<WatchStruct*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WatchStruct)));

	if (tWatch == nullptr)
		return nullptr;

	Watcher* pWatch = mem::akaNew<Watcher>(AllocatorMemoryType::Persistent, AllocatorCategory::Default);
	tWatch->watcher = pWatch;

	pWatch->mDirHandle = CreateFileW(
		szDirectory,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		nullptr
	);
	if (pWatch->mDirHandle != INVALID_HANDLE_VALUE && CreateIoCompletionPort(pWatch->mDirHandle, mIOCP, 0, 1))
	{
		pWatch->mNotifyFilter = mNotifyFilter;
		pWatch->mIsRecursive = recursive;

		if (RefreshWatch(tWatch))
		{
			return tWatch;
		}
	}
	CloseHandle(pWatch->mDirHandle);
	mem::akaDelete(pWatch);
	HeapFree(GetProcessHeap(), 0, tWatch);
	return nullptr;
}

struct FileWatcher::OSData
{
	OSData() : 
		m_iocp(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1)),
		m_thread(&FileWatcher::OSData::run, this),
		m_initOK(m_iocp && m_iocp != INVALID_HANDLE_VALUE)
	{
	}
	~OSData()
	{
		// Stop the thread when we finish
		m_initOK = false;
		if (m_iocp && m_iocp != INVALID_HANDLE_VALUE)
		{
			PostQueuedCompletionStatus(m_iocp, 0, reinterpret_cast<ULONG_PTR>(this), NULL);
		}
		if (m_thread.joinable())
			m_thread.join();
		for (WatchStruct* watch : m_watches)
		{
			DestroyWatch(watch);
		}
		if (m_iocp)
			CloseHandle(m_iocp);
	}

	void run()
	{
		do {
			if (m_initOK && !m_watches.empty()) {
				DWORD numOfBytes = 0;
				OVERLAPPED* ov = NULL;
				ULONG_PTR compKey = 0;
				BOOL res = FALSE;

				while ((res = GetQueuedCompletionStatus(m_iocp, &numOfBytes, &compKey, &ov, INFINITE)) != FALSE)
				{
					if (compKey != 0 && compKey == reinterpret_cast<ULONG_PTR>(this))
					{
						break;
					}
					else
					{
						WatchCallback(numOfBytes, ov);
					}
				}
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
			}
		} while (m_initOK);
	}
	HashSet<WatchStruct*> m_watches;

	HANDLE m_iocp;
	std::thread m_thread;
	bool m_initOK;
};

FileWatcher::FileWatcher() :
	m_osData(mem::akaNew<OSData>(AllocatorMemoryType::Persistent, AllocatorCategory::Default))
{
}
FileWatcher::~FileWatcher()
{
	mem::akaDelete(m_osData);
}

bool FileWatcher::addWatch(const Path& path, bool recursive, FileWatchEvent callback)
{
	StringWide wstr = Utf8ToWchar(path.cstr());
	WatchStruct* tWatch = CreateWatch(
		wstr.cstr(), 
		recursive, 
		FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_LAST_WRITE |
		FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
		FILE_NOTIFY_CHANGE_SIZE,
		m_osData->m_iocp
	);

	if (tWatch == nullptr)
	{
		Logger::error("Failed setup watch: ", GetLastErrorAsString());
		return false;
	}

	tWatch->watcher->mFileWatcher = this;
	tWatch->watcher->mFileWatchEvent = callback;
	tWatch->watcher->mDirName = path;

	m_osData->m_watches.insert(tWatch);

	Logger::debug("Start watching folder: ", path);
	return true;
}
void FileWatcher::removeWatch(const Path& path)
{
	for (WatchStruct* watch : m_osData->m_watches)
	{
		if (path == watch->watcher->mDirName)
		{
			m_osData->m_watches.erase(watch);
			DestroyWatch(watch);
			return;
		}
	}
}

}

#endif