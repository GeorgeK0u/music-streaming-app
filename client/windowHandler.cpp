#include "windowHandler.h"
#include <stdexcept>

WindowHandler::WindowHandler()
{
	this->hwnd = NULL;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// handle window messages
	switch (uMsg)
    {
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+3));
			EndPaint(hwnd, &ps);
			break;
		}
		default:
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
    }
	return 0;
}

HWND WindowHandler::CreateShowWindow(HINSTANCE hInstance)
{
	// create window class
	char wndClassName[] = "Window"; 
	this->wc = { };
	this->wc.lpfnWndProc = WindowProc;
	this->wc.hInstance = hInstance;
	this->wc.lpszClassName = wndClassName;
	// register window class
	BOOL regWndClassOk = RegisterClassA(&this->wc);
	if (!regWndClassOk)
	{
		throw std::runtime_error("Failed to register window class");
	}
	// create window
	this->hwnd = CreateWindowExA(0, wndClassName, "Window", WS_OVERLAPPEDWINDOW, 0, 0, 200, 200, NULL, NULL, hInstance, NULL);
	if (hwnd == NULL)
	{
		throw std::runtime_error("Failed to create a window");
	}
	ShowWindow(this->hwnd, SW_SHOW);
	return this->hwnd;
}

void WindowHandler::Release()
{
	if (this->hwnd != NULL)
	{
		this->hwnd = NULL;
	}
}
