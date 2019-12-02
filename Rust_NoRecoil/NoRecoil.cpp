#include "NoRecoil.h"
#include <cmath>
#include <iostream>

void NoRecoil::init() {
	initGuns();
	if (antiRecoilThread == nullptr) {
		noRecoilActiveMutex.lock();
		leftClickDownMutex.lock();
		cancelRecoilSleepMutex.lock();
		antiRecoilThread = new std::thread{ antiRecoilThreadFunction, GetCurrentThreadId() };
	}
}

void NoRecoil::initGuns() {
	int *scope_x_stance_y;

	// AK_47
	guns[0].RPM = 450;
	guns[0].shots = 30;
	guns[0].millisecondsRecoilCooldown = 500;
	guns[0].fullyAutomatic = true;
	scope_x_stance_y = new int[30]{ -28,5,-44,-35,0,13,24,30,34,34,30,24,13,2,-12,-23,-29,-35,-36,-37,-33,-28,-19,-9,9,29,38,40,31,12 };
	for (int i = 0; i < NUMBER_OF_GUN_SCOPE; ++i) {
		for (int j = 0; j < NUMBER_OF_STANCE; ++j) {
			guns[0].recoilOffsetX[i][j] = scope_x_stance_y;
		}
	}
	scope_x_stance_y = new int[30]{ 40,37,33,30,28,20,21,16,10,7,8,16,19,25,25,27,25,23,21,13,7,3,11,17,19,23,24,19,17,12 };
	for (int i = 0; i < NUMBER_OF_GUN_SCOPE; ++i) {
		for (int j = 0; j < NUMBER_OF_STANCE; ++j) {
			guns[0].recoilOffsetY[i][j] = scope_x_stance_y;
		}
	}
	// SAR
	guns[1].RPM = 343;
	guns[1].shots = 16;
	guns[1].millisecondsRecoilCooldown = 500;
	guns[1].fullyAutomatic = false;
	scope_x_stance_y = new int[30]{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	for (int i = 0; i < NUMBER_OF_GUN_SCOPE; ++i) {
		for (int j = 0; j < NUMBER_OF_STANCE; ++j) {
			guns[1].recoilOffsetX[i][j] = scope_x_stance_y;
		}
	}
	scope_x_stance_y = new int[30]{ 52,52,40,40,40,40,40,40,40,40,40,40,40,40,40,40 };
	for (int i = 0; i < NUMBER_OF_GUN_SCOPE; ++i) {
		for (int j = 0; j < NUMBER_OF_STANCE; ++j) {
			guns[1].recoilOffsetY[i][j] = scope_x_stance_y;
		}
	}

	millisecondsBetweenShots = std::lround(60000.0 / guns[0].RPM);
	// Otherwise GetCursorPos returns "wrong" positions
	SetProcessDPIAware();
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
	cancelRecoilSleepMutex.try_lock();
	cancelRecoilSleepMutex.unlock();
	cancelRecoilResetMutex.try_lock();
	cancelRecoilResetMutex.unlock();
	shotCountMutex.try_lock();
	shotCountMutex.unlock();
}

void NoRecoil::moveMouseWithDelta(int dX, int dY) {
	INPUT input = { 0 };
	POINT mousePosition;
	GetCursorPos(&mousePosition);

	std::cout << "( " << mousePosition.x << "," << mousePosition.y << ") + ( " << dX << "," << dY << ")\n";

	input.type = INPUT_MOUSE;
	input.mi.dx = std::lround(((mousePosition.x + dX) * 65535.0) / GetSystemMetrics(SM_CXSCREEN));
	input.mi.dy = std::lround(((mousePosition.y + dY) * 65535.0) / GetSystemMetrics(SM_CYSCREEN));
	// MOUSEEVENTF_MOVE
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE;
	SendInput(1, &input, sizeof(input));
}

void NoRecoil::antiRecoilThreadFunction(DWORD main_thread_id) {
	std::chrono::milliseconds timeout;
	int recoilOffsetX, recoilOffsetY;
	int localShotCount, localPrevErrorX, localPrevErrorY, offsetErrorX, offsetErrorY;

	while (true) {
		noRecoilActiveMutex.lock();
		noRecoilActiveMutex.unlock();

		// CHECK IF leftClickDown
		leftClickDownMutex.lock();
		leftClickDownMutex.unlock();

		if (noRecoilActive) {
			offsetErrorX = offsetError(gen);
			offsetErrorY = offsetError(gen);
			// CHECK bullet count
			shotCountMutex.lock();
			localShotCount = (shotCount++) % guns[currentGun].shots;
			localPrevErrorX = prevErrorX;
			localPrevErrorY = prevErrorY;
			prevErrorX = offsetErrorX;
			prevErrorY = offsetErrorY;
			shotCountMutex.unlock();
			recoilOffsetX = guns[currentGun].recoilOffsetX[currentGunScope][currentStance][localShotCount]
				- localPrevErrorX + offsetErrorX;
			recoilOffsetY = guns[currentGun].recoilOffsetY[currentGunScope][currentStance][localShotCount]
				- localPrevErrorY + offsetErrorY;

			std::cout << "currentGun: " << currentGun << ", currentGunScope: " << currentGunScope <<
				", currentStance: " << currentStance << ", shotCount: " << localShotCount << "\n";

			moveMouseWithDelta(recoilOffsetX, recoilOffsetY);
			if (!guns[currentGun].fullyAutomatic) {
				// Send virtual left click UP signal in order to stop continuous recoil compensation
				INPUT Input = { 0 };
				Input.type = INPUT_MOUSE;                 // Let input know we are using the mouse.
				Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;    // We are setting left mouse button up.
				SendInput(1, &Input, sizeof(INPUT));      // Send the input.
			}

			// TIME BETWEEN SHOTS
			timeout = std::chrono::milliseconds{ millisecondsBetweenShots };
			if (cancelRecoilSleepMutex.try_lock_for(timeout)) {
				cancelRecoilSleepMutex.unlock();
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
	if (cancelRecoilResetMutex.try_lock_for(std::chrono::milliseconds{ millisecondsRecoilCooldown })) {
		cancelRecoilResetMutex.unlock();
	}
	else {
		shotCountMutex.lock();
		prevErrorX = prevErrorY = shotCount = 0;
		shotCountMutex.unlock();
	}
}

void NoRecoil::processMouseInput(WPARAM wParam, PMSLLHOOKSTRUCT pMouseStruct) {
	switch (wParam) {
	case WM_LBUTTONDOWN:
		// DOWN: left click
		// Cancel resetting recoil
		cancelRecoilResetMutex.try_lock();
		cancelRecoilResetMutex.unlock();
		if (recoilResetThread.valid()) {
			recoilResetThread.wait();
		}
		// Let the antiRecoilThread run
		leftClickDownMutex.try_lock();
		leftClickDownMutex.unlock();
		break;
	case WM_LBUTTONUP:
		// UP: left click
		leftClickDownMutex.lock();
		// Start resetting recoil if we were not the thread owning 'cancelRecoilResetMutex'
		if (cancelRecoilResetMutex.lock()) {
			recoilResetThread = std::async(std::launch::async, recoilResetThreadFunction, guns[currentGun].millisecondsRecoilCooldown);
		}
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
			// UP: digit
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
					millisecondsBetweenShots = std::lround(60000.0 / guns[currentGun].RPM);
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

std::random_device NoRecoil::rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 NoRecoil::gen(rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<int> NoRecoil::offsetError(-OFFSET_ERROR_BOUND, OFFSET_ERROR_BOUND);
int NoRecoil::prevErrorX = 0, NoRecoil::prevErrorY = 0;

NoRecoil::Gun NoRecoil::guns[NUMBER_OF_GUN];
NoRecoil::GunIndex NoRecoil::currentGun = NoRecoil::GunIndex::AK_47;
NoRecoil::GunScopeIndex NoRecoil::currentGunScope = NoRecoil::GunScopeIndex::NONE;
NoRecoil::StanceIndex NoRecoil::currentStance = NoRecoil::StanceIndex::STAND;

int NoRecoil::shotCount = 0;
long NoRecoil::millisecondsBetweenShots;
// Must start as 'false', because initially noRecoilActiveMutex
// is locked to prevent antiRecoilThread from looping (running)!
bool NoRecoil::noRecoilActive = false;

bool NoRecoil::acceptGunNumber = false;
int NoRecoil::enteredGunNumber;

std::thread* NoRecoil::antiRecoilThread = nullptr;
std::future<void> NoRecoil::recoilResetThread;
SafeMutex NoRecoil::noRecoilActiveMutex;
SafeMutex NoRecoil::leftClickDownMutex;
SafeTimedMutex NoRecoil::cancelRecoilSleepMutex;
SafeTimedMutex NoRecoil::cancelRecoilResetMutex;

SafeMutex NoRecoil::shotCountMutex;
