#include "SafeMutex.h"

void SafeMutex::lock() {
	if (ownerThreadID != GetCurrentThreadId()) {
		mutex::lock();
	}
	ownerThreadID = GetCurrentThreadId();
}

bool SafeMutex::try_lock() {
	if (mutex::try_lock()) {
		ownerThreadID = GetCurrentThreadId();
		return true;
	}
	return false;
}

void SafeMutex::unlock() {
	ownerThreadID = 0;
	mutex::unlock();
}

DWORD SafeMutex::getOwnerThreadID() {
	return ownerThreadID;
}
