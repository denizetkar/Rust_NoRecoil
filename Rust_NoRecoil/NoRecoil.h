#ifndef NO_RECOIL_H
#define NO_RECOIL_H

#include "MyHook.h"
#include "SafeMutex.h"
#include "SafeTimedMutex.h"
#include <Windows.h>
#include <thread>
#include <mutex>
#include <future>
#include <chrono>
#include <random>

#define NUMBER_OF_GUN 2
#define NUMBER_OF_GUN_SCOPE 5
#define NUMBER_OF_STANCE 2

#define OFFSET_ERROR_BOUND 5

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
		bool fullyAutomatic;
		// Dimensions: [#gun_scopes][#stances] ([#shots])
		int *recoilOffsetX[NUMBER_OF_GUN_SCOPE][NUMBER_OF_STANCE];
		int *recoilOffsetY[NUMBER_OF_GUN_SCOPE][NUMBER_OF_STANCE];
	};

	static std::random_device rd;  //Will be used to obtain a seed for the random number engine
	static std::mt19937 gen; //Standard mersenne_twister_engine seeded with rd()
	static std::uniform_int_distribution<int> offsetError;
	static int prevErrorX, prevErrorY;

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
	static SafeMutex noRecoilActiveMutex;
	static SafeMutex leftClickDownMutex;
	static SafeTimedMutex cancelRecoilSleepMutex;
	static SafeTimedMutex cancelRecoilResetMutex;

	static std::mutex shotCountMutex;
public:
	static void init();

protected:
	static void initGuns();

public:
	static void startHooks();

protected:
	static void destroy();
	static void moveMouseWithDelta(int, int);

	static void antiRecoilThreadFunction(DWORD);
	static void recoilResetThreadFunction(long);

	static void processMouseInput(WPARAM, PMSLLHOOKSTRUCT);
	static void processKeyboardInput(WPARAM, PKBDLLHOOKSTRUCT);
};

#endif
