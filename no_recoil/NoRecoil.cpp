#include "NoRecoil.h"
#include <cmath>
#include <iostream>

void NoRecoil::init() {
	initGuns();
	if (antiRecoilThread == nullptr) {
		noRecoilActiveMutex.lock();
		leftClickDownMutex.lock();
		cancelRecoilSleep.lock();
		antiRecoilThread = new std::thread{ antiRecoilThreadFunction, GetCurrentThreadId() };
	}
}

void NoRecoil::initGuns() {
	int *scope_x_stance_y;

	// AK_47
	guns[0].RPM = 450;
	guns[0].shots = 30;
	guns[0].millisecondsRecoilCooldown = 500;
	scope_x_stance_y = new int[30]{ 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29 };
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 2; ++j) {
			guns[0].recoilOffsetX[i][j] = scope_x_stance_y;
		}
	}
	scope_x_stance_y = new int[30]{ 10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10 };
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 2; ++j) {
			guns[0].recoilOffsetY[i][j] = scope_x_stance_y;
		}
	}
	// SAR
	guns[1].RPM = 343;
	guns[1].shots = 16;
	guns[1].millisecondsRecoilCooldown = 500;
	scope_x_stance_y = new int[30]{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 2; ++j) {
			guns[1].recoilOffsetX[i][j] = scope_x_stance_y;
		}
	}
	scope_x_stance_y = new int[30]{ 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5 };
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 2; ++j) {
			guns[1].recoilOffsetY[i][j] = scope_x_stance_y;
		}
	}

	millisecondsBetweenShots = std::lround(1000.0 / guns[0].RPM);
	// Otherwise GetCursorPos returns "wrong" positions
	SetProcessDPIAware();
}

void NoRecoil::antiRecoilThreadFunction(DWORD main_thread_id) {
	POINT mousePosition;
	std::chrono::milliseconds timeout;
	int recoilOffsetX, recoilOffsetY;
	int localShotCount;

	while (true) {
		noRecoilActiveMutex.lock();
		noRecoilActiveMutex.unlock();

		// CHECK IF leftClickDown
		leftClickDownMutex.lock();
		leftClickDownMutex.unlock();

		if (noRecoilActive) {
			// TODO: CHECK bullet count!!!!!!!!!!!!!
			shotCountMutex.lock();
			localShotCount = (shotCount++) % guns[currentGun].shots;
			shotCountMutex.unlock();
			recoilOffsetX = guns[currentGun].recoilOffsetX[currentGunScope][currentStance][localShotCount];
			recoilOffsetY = guns[currentGun].recoilOffsetY[currentGunScope][currentStance][localShotCount];
			GetCursorPos(&mousePosition);
			SetCursorPos(mousePosition.x + recoilOffsetX, mousePosition.y + recoilOffsetY);
			std::cout << "currentGun: " << currentGun << ", currentGunScope: " << currentGunScope <<
				", currentStance: " << currentStance << ", shotCount: " << localShotCount << "\n";
			std::cout << "(" << mousePosition.x << ", " << mousePosition.y << ") + ( " << recoilOffsetX << "," << recoilOffsetY << ")\n";

			// TIME BETWEEN SHOTS
			timeout = std::chrono::milliseconds{ millisecondsBetweenShots };
			if (cancelRecoilSleep.try_lock_for(timeout)) {
				cancelRecoilSleep.unlock();
			}
		}
	}
	// MUST NEVER REACH HERE
	// WARN THE USER !!!!!!!!!!!
	for (int i = 0; i < 5; ++i) {
		std::cout << "\a";
		Sleep(100);
	}
	// Let the main thread out of the message loop
	PostThreadMessage(main_thread_id, WM_QUIT, NULL, NULL);
	antiRecoilThread = nullptr;
}

void NoRecoil::recoilResetThreadFunction(long millisecondsRecoilCooldown) {
	if (cancelRecoilReset.try_lock_for(std::chrono::milliseconds{ millisecondsRecoilCooldown })) {
		cancelRecoilReset.unlock();
	}
	else {
		shotCountMutex.lock();
		shotCount = 0;
		shotCountMutex.unlock();
	}
}

void NoRecoil::startHooks() {
	MyHook::Instance().InstallHooks();
	MyHook::Instance(processMouseInput, processKeyboardInput, destroy).Messsages();
}

void NoRecoil::destroy() {
	noRecoilActiveMutex.try_lock();
	noRecoilActiveMutex.unlock();
	leftClickDownMutex.try_lock();
	leftClickDownMutex.unlock();
	cancelRecoilSleep.try_lock();
	cancelRecoilSleep.unlock();
	cancelRecoilReset.try_lock();
	cancelRecoilReset.unlock();
}

void NoRecoil::processMouseInput(WPARAM wParam, PMSLLHOOKSTRUCT pMouseStruct) {
	switch (wParam) {
	case WM_LBUTTONDOWN:
		// DOWN: left click
		// Cancel resetting recoil
		cancelRecoilReset.try_lock();
		cancelRecoilReset.unlock();
		if (recoilResetThread.valid()) {
			recoilResetThread.wait();
		}
		// Let the antiRecoilThread run
		cancelRecoilSleep.unlock();
		leftClickDownMutex.unlock();
		cancelRecoilSleep.lock();
		break;
	case WM_LBUTTONUP:
		// UP: left click
		leftClickDownMutex.lock();
		// Start resetting recoil
		cancelRecoilReset.lock();
		recoilResetThread = std::async(std::launch::async, recoilResetThreadFunction, guns[currentGun].millisecondsRecoilCooldown);
		break;
	default:
		break;
	}
}

void NoRecoil::processKeyboardInput(WPARAM wParam, PKBDLLHOOKSTRUCT kbdStruct) {
	int digit;
	// See for virtual key codes:
	// https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes
	if (wParam == WM_KEYDOWN) {
		switch (kbdStruct->vkCode) {
		case VK_LCONTROL:
			// DOWN: left ctrl
			currentStance = StanceIndex::CROUCH;
			break;
		}
	}
	else if (wParam == WM_KEYUP) {
		switch (kbdStruct->vkCode) {
		case VK_LCONTROL:
			// UP: left ctrl
			currentStance = StanceIndex::STAND;
			break;
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
			digit = kbdStruct->vkCode - 0x30;
			if (acceptGunNumber) {
				enteredGunNumber *= 10;
				enteredGunNumber += digit;
			}
			break;
		case 0x42:
			// UP: b
			acceptGunNumber = false;
			if (noRecoilActive) {
				// DEACTIVATE
				noRecoilActiveMutex.lock();
			}
			else {
				// ACTIVATE
				noRecoilActiveMutex.unlock();
				enteredGunNumber = 0;
				std::cout << "\a";
			}
			noRecoilActive = !noRecoilActive;
			break;
		case 0x4E:
			// UP: n
			if (noRecoilActive) {

				if (acceptGunNumber) {
					enteredGunNumber -= 1;
					if (enteredGunNumber < 0) {
						enteredGunNumber = 0;
					}
					else if (enteredGunNumber >= NUMBER_OF_GUN) {
						enteredGunNumber = NUMBER_OF_GUN - 1;
					}
					currentGun = (GunIndex)enteredGunNumber;
					millisecondsBetweenShots = std::lround(1000.0 / guns[currentGun].RPM);
					enteredGunNumber = 0;
				}
				else {
					std::cout << "\a";
				}

				acceptGunNumber = !acceptGunNumber;
			}
			break;
		}
	}
}

NoRecoil::Gun NoRecoil::guns[NUMBER_OF_GUN];
NoRecoil::GunIndex NoRecoil::currentGun;
NoRecoil::GunScopeIndex NoRecoil::currentGunScope;
NoRecoil::StanceIndex NoRecoil::currentStance;
int NoRecoil::shotCount = 0;
long NoRecoil::millisecondsBetweenShots;
// Must start as 'false', otherwise the main thread will DEADLOCK
// after receiving state transition command (will call lock on runMutex)!
bool NoRecoil::noRecoilActive = false;

bool NoRecoil::acceptGunNumber = false;
int NoRecoil::enteredGunNumber;

std::thread* NoRecoil::antiRecoilThread = nullptr;
std::future<void> NoRecoil::recoilResetThread;
std::mutex NoRecoil::noRecoilActiveMutex;
std::mutex NoRecoil::leftClickDownMutex;
std::timed_mutex NoRecoil::cancelRecoilSleep;
std::timed_mutex NoRecoil::cancelRecoilReset;

std::mutex NoRecoil::shotCountMutex;
