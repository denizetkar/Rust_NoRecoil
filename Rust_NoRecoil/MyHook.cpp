#include "MyHook.h"

int MyHook::Messsages() {
	while (msg.message != WM_QUIT) { //while we do not close our application
		//PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)
		if (GetMessage(&msg, NULL, NULL, NULL)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//Sleep(1);
	}
	if (destroy_callback) {
		destroy_callback();
	}
	UninstallHooks(); //if we close, let's uninstall our hook
	return (int)msg.wParam; //return the messages
}

void MyHook::InstallHooks() {
	/*
	SetWindowHookEx(
	WM_MOUSE_LL = mouse low level hook type,
	MyMouseCallback = our callback function that will deal with system messages about mouse
	NULL, 0);

	c++ note: we can check the return SetWindowsHookEx like this because:
	If it return NULL, a NULL value is 0 and 0 is false.
	*/
	mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, MyMouseCallback, NULL, NULL);
	keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, MyKeyboardCallback, NULL, NULL);
}

void MyHook::UninstallHooks() {
	/*
	uninstall our hook using the hook handle
	*/
	UnhookWindowsHookEx(mouse_hook);
	UnhookWindowsHookEx(keyboard_hook);
}

HHOOK MyHook::getMouseHook() {
	return mouse_hook;
}

MyHook::HOOK_MOUSE_CALLBACK MyHook::getHookMouseCallBack() {
	return hook_mouse_callback;
}

HHOOK MyHook::getKeyboardHook() {
	return keyboard_hook;
}

MyHook::HOOK_KEYBOARD_CALLBACK MyHook::getHookKeyboardCallBack() {
	return hook_keyboard_callback;
}

LRESULT CALLBACK MyMouseCallback(int nCode, WPARAM wParam, LPARAM lParam) {
	MSLLHOOKSTRUCT * pMouseStruct = (MSLLHOOKSTRUCT *)lParam; // WH_MOUSE_LL struct
															  /*
															  nCode, this parameters will determine how to process a message
															  This callback in this case only have information when it is 0 (HC_ACTION): wParam and lParam contain info

															  wParam is about WINDOWS MESSAGE, in this case MOUSE messages.
															  lParam is information contained in the structure MSLLHOOKSTRUCT
															  */

	if (nCode == HC_ACTION) { // we have information in wParam/lParam ? If yes, let's check it:

		MyHook::HOOK_MOUSE_CALLBACK hook_callback = MyHook::Instance().getHookMouseCallBack();
		if (hook_callback) {
			hook_callback(wParam, pMouseStruct);
		}

	}

	/*
	Every time that the nCode is less than 0 we need to CallNextHookEx:
	-> Pass to the next hook
	MSDN: Calling CallNextHookEx is optional, but it is highly recommended;
	otherwise, other applications that have installed hooks will not receive hook notifications and may behave incorrectly as a result.
	*/
	return CallNextHookEx(MyHook::Instance().getMouseHook(), nCode, wParam, lParam);
}

LRESULT CALLBACK MyKeyboardCallback(int nCode, WPARAM wParam, LPARAM lParam) {
	KBDLLHOOKSTRUCT * kbdStruct = (KBDLLHOOKSTRUCT *)lParam;

	if (nCode == HC_ACTION) { // we have information in wParam/lParam ? If yes, let's check it:

		MyHook::HOOK_KEYBOARD_CALLBACK hook_callback = MyHook::Instance().getHookKeyboardCallBack();
		if (hook_callback) {
			hook_callback(wParam, kbdStruct);
		}

	}

	/*
	Every time that the nCode is less than 0 we need to CallNextHookEx:
	-> Pass to the next hook
	MSDN: Calling CallNextHookEx is optional, but it is highly recommended;
	otherwise, other applications that have installed hooks will not receive hook notifications and may behave incorrectly as a result.
	*/
	return CallNextHookEx(MyHook::Instance().getKeyboardHook(), nCode, wParam, lParam);
}
