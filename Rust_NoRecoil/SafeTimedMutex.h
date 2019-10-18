#ifndef SAFE_TIMED_MUTEX_H
#define SAFE_TIMED_MUTEX_H

#include <Windows.h>
#include <mutex>
#include <chrono>

class SafeTimedMutex : public std::timed_mutex {
protected:
	DWORD ownerThreadID = NULL;
	std::mutex innerMutex;
public:
	bool lock();
	bool try_lock();
	void unlock();

	bool try_lock_for(const std::chrono::milliseconds& _Rel_time);

	template<class _Clock,
		class _Duration>
		bool try_lock_until(
			const std::chrono::time_point<_Clock, _Duration>& _Abs_time) = delete;
};

#endif
