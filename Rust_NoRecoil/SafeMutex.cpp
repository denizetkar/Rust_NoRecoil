#include "SafeMutex.h"

bool SafeMutex::lock() {
	innerMutex.lock();
	DWORD ownerThreadID_local = ownerThreadID;
	innerMutex.unlock();
	if (ownerThreadID_local != GetCurrentThreadId()) {
		mutex::lock();
		innerMutex.lock();
		ownerThreadID = GetCurrentThreadId();
		innerMutex.unlock();
		return true;
	}
	return false;
}

bool SafeMutex::try_lock() {
	if (mutex::try_lock()) {
		innerMutex.lock();
		ownerThreadID = GetCurrentThreadId();
		innerMutex.unlock();
		return true;
	}
	return false;
}

void SafeMutex::unlock() {
	innerMutex.lock();
	if (ownerThreadID == GetCurrentThreadId()) {
		ownerThreadID = NULL;
		innerMutex.unlock();
		mutex::unlock();
	}
	else innerMutex.unlock();
}
