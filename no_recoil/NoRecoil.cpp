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
	scope_x_stance_y = new int[30]{ -41,5,-60,-52,0,20,28,46,58,48,52,30,28,-8,-20,-30,-52,-54,-60,-56,-52,-44,-42,18,0,0,0,0,0,0 };
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 2; ++j) {
			guns[0].recoilOffsetX[i][j] = scope_x_stance_y;
		}
	}
	scope_x_stance_y = new int[30]{ 58,56,50,46,40,26,24,34,16,12,14,16,36,34,42,46,36,40,30,20,16,8,8,12,26,0,0,0,0,0 };
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 2; ++j) {
			guns[0].recoilOffsetY[i][j] = scope_x_stance_y;
		}
	}
	// SAR
	guns[1].RPM = 343;
	guns[1].shots = 16;
	guns[1].millisecondsRecoilCooldown = 500;
	guns[1].fullyAutomatic = false;
	scope_x_stance_y = new int[30]{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 2; ++j) {
			guns[1].recoilOffsetX[i][j] = scope_x_stance_y;
		}
	}
	scope_x_stance_y = new int[30]{ 52,52,40,40,40,40,40,40,40,40,40,40,40,40,40,40 };
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 2; ++j) {
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
	int localShotCount;

	while (true) {
		noRecoilActiveMutex.lock();
		noRecoilActiveMutex.unlock();

		// CHECK IF leftClickDown
		leftClickDownMutex.lock();
		leftClickDownMutex.unlock();

		if (noRecoilActive) {
			// CHECK bullet count
			shotCountMutex.lock();
			localShotCount = (shotCount++) % guns[currentGun].shots;
			shotCountMutex.unlock();
			recoilOffsetX = guns[currentGun].recoilOffsetX[currentGunScope][currentStance][localShotCount];
			recoilOffsetY = guns[currentGun].recoilOffsetY[currentGunScope][currentStance][localShotCount];

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
		shotCount = 0;
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
		cancelRecoilSleepMutex.unlock();
		leftClickDownMutex.unlock();
		cancelRecoilSleepMutex.lock();
		break;
	case WM_LBUTTONUP:
		// UP: left click
		leftClickDownMutex.lock();
		if (cancelRecoilResetMutex.getOwnerThreadID() != GetCurrentThreadId()) {
			// Start resetting recoil
			cancelRecoilResetMutex.lock();
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
SafeMutex NoRecoil::leftClickDownMutex;
std::timed_mutex NoRecoil::cancelRecoilSleepMutex;
SafeTimedMutex NoRecoil::cancelRecoilResetMutex;

std::mutex NoRecoil::shotCountMutex;
