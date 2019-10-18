#ifndef SAFE_MUTEX_H
#define SAFE_MUTEX_H

#include <Windows.h>
#include <mutex>

class SafeMutex : public std::mutex {
protected:
	DWORD ownerThreadID = NULL;
	std::mutex innerMutex;
public:
	bool lock();
	bool try_lock();
	void unlock();
};

#endif
