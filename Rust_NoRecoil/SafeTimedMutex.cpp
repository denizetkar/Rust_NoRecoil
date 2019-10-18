#include "SafeTimedMutex.h"

bool SafeTimedMutex::lock() {
	innerMutex.lock();
	DWORD ownerThreadID_local = ownerThreadID;
	innerMutex.unlock();
	if (ownerThreadID_local != GetCurrentThreadId()) {
		timed_mutex::lock();
		innerMutex.lock();
		ownerThreadID = GetCurrentThreadId();
		innerMutex.unlock();
		return true;
	}
	return false;
}

bool SafeTimedMutex::try_lock() {
	if (timed_mutex::try_lock()) {
		innerMutex.lock();
		ownerThreadID = GetCurrentThreadId();
		innerMutex.unlock();
		return true;
	}
	return false;
}

void SafeTimedMutex::unlock() {
	innerMutex.lock();
	if (ownerThreadID == GetCurrentThreadId()) {
		ownerThreadID = NULL;
		innerMutex.unlock();
		timed_mutex::unlock();
	} else innerMutex.unlock();
}

bool SafeTimedMutex::try_lock_for(const std::chrono::milliseconds& _Rel_time)
{	// try to lock for duration
	if (timed_mutex::try_lock_until(std::chrono::steady_clock::now() + _Rel_time)) {
		innerMutex.lock();
		ownerThreadID = GetCurrentThreadId();
		innerMutex.unlock();
		return true;
	}
	return false;
}
