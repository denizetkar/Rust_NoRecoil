#ifndef MY_HOOK_H
#define MY_HOOK_H

#include <Windows.h>

class MyHook {
	MyHook() : msg {}, mouse_hook { nullptr }, hook_mouse_callback{ nullptr }, keyboard_hook{ nullptr },
		hook_keyboard_callback{ nullptr }, destroy_callback{ nullptr } {}

public:
	using HOOK_MOUSE_CALLBACK = void (*)(WPARAM, PMSLLHOOKSTRUCT);
	using HOOK_KEYBOARD_CALLBACK = void(*)(WPARAM, PKBDLLHOOKSTRUCT);
	using DESTROY_CALLBACK = void(*)();

	//single ton
	static MyHook& Instance(
		HOOK_MOUSE_CALLBACK _hook_mouse_callback = NULL, 
		HOOK_KEYBOARD_CALLBACK _hook_keyboard_callback = NULL,
		DESTROY_CALLBACK _destroy_callback = NULL) {
		static MyHook myHook;
		if (_hook_mouse_callback) {
			myHook.hook_mouse_callback = _hook_mouse_callback;
		}
		if (_hook_keyboard_callback) {
			myHook.hook_keyboard_callback = _hook_keyboard_callback;
		}
		if (_destroy_callback) {
			myHook.destroy_callback = _destroy_callback;
		}
		return myHook;
	}
	void InstallHooks(); // function to install our hook
	void UninstallHooks(); // function to uninstall our hook

	HHOOK getMouseHook();
	HOOK_MOUSE_CALLBACK getHookMouseCallBack();
	HHOOK getKeyboardHook();
	HOOK_KEYBOARD_CALLBACK getHookKeyboardCallBack();

	MSG msg; // struct with information about all messages in our queue
	int Messsages(); // function to "deal" with our messages 
protected:
	HHOOK mouse_hook; // handle to the hook	
	HOOK_MOUSE_CALLBACK hook_mouse_callback;
	HHOOK keyboard_hook; // handle to the hook	
	HOOK_KEYBOARD_CALLBACK hook_keyboard_callback;

	DESTROY_CALLBACK destroy_callback;
};
LRESULT CALLBACK MyMouseCallback(int nCode, WPARAM wParam, LPARAM lParam); //callback declaration
LRESULT CALLBACK MyKeyboardCallback(int nCode, WPARAM wParam, LPARAM lParam); //callback declaration

#endif
