#include "SafeTimedMutex.h"

void SafeTimedMutex::lock() {
	if (ownerThreadID != GetCurrentThreadId()) {
		timed_mutex::lock();
	}
	ownerThreadID = GetCurrentThreadId();
}

bool SafeTimedMutex::try_lock() {
	if (timed_mutex::try_lock()) {
		ownerThreadID = GetCurrentThreadId();
		return true;
	}
	return false;
}

void SafeTimedMutex::unlock() {
	ownerThreadID = NULL;
	timed_mutex::unlock();
}

DWORD SafeTimedMutex::getOwnerThreadID() {
	return ownerThreadID;
}
