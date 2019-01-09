#ifndef NO_RECOIL_H
#define NO_RECOIL_H

#include <Windows.h>
#include "MyHook.h"
#include <thread>
#include <mutex>
#include <future>
#include <chrono>

#define NUMBER_OF_GUN 2
#define NUMBER_OF_GUN_SCOPE 5
#define NUMBER_OF_STANCE 2

class NoRecoil {

protected:
	NoRecoil() {}

	// ALWAYS START FROM 0 AND INCREASE BY 1
	enum GunIndex {
		AK_47 = 0, SAR = 1
	};
	// ALWAYS START FROM 0 AND INCREASE BY 1
	enum GunScopeIndex {
		NONE = 0, SIMPLE = 1, HOLO = 2, _4X = 3, _8X = 4
	};
	// ALWAYS START FROM 0 AND INCREASE BY 1
	enum StanceIndex {
		STAND = 0, CROUCH = 1
	};
	struct Gun {
		int RPM;
		int shots;
		long millisecondsRecoilCooldown;
		// Dimensions: [#gun_scopes][#stances] ([#shots])
		int *recoilOffsetX[NUMBER_OF_GUN_SCOPE][NUMBER_OF_STANCE];
		int *recoilOffsetY[NUMBER_OF_GUN_SCOPE][NUMBER_OF_STANCE];
	};

	// Dimension: [#guns]
	static Gun guns[NUMBER_OF_GUN];
	static GunIndex currentGun;
	static GunScopeIndex currentGunScope;
	static StanceIndex currentStance;
	static long millisecondsBetweenShots;
	static int shotCount;
	static bool noRecoilActive;

	static bool acceptGunNumber;
	static int enteredGunNumber;

	static std::thread *antiRecoilThread;
	static std::future<void> recoilResetThread;
	static std::mutex noRecoilActiveMutex;
	static std::mutex leftClickDownMutex;
	static std::timed_mutex cancelRecoilSleep;
	static std::timed_mutex cancelRecoilReset;

	static std::mutex shotCountMutex;
public:
	static void init();
	static void initGuns();

protected:
	static void antiRecoilThreadFunction(DWORD);
	static void recoilResetThreadFunction(long);

public:
	static void startHooks();
	static void destroy();

protected:
	static void processMouseInput(WPARAM, PMSLLHOOKSTRUCT);
	static void processKeyboardInput(WPARAM, PKBDLLHOOKSTRUCT);
};

#endif
