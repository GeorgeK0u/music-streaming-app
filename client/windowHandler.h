#pragma once
#include <windows.h>

class WindowHandler
{
	public:
		WindowHandler();
		HWND CreateShowWindow(HINSTANCE hInstance);
		void Release();

	private:
		HWND hwnd;
		WNDCLASSA wc;
};
