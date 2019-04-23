#ifndef SAFE_MUTEX_H
#define SAFE_MUTEX_H

#include <Windows.h>
#include <mutex>

class SafeMutex : public std::mutex {
protected:
	DWORD ownerThreadID = NULL;
public:
	void lock();
	bool try_lock();
	void unlock();

	DWORD getOwnerThreadID();
};

#endif
