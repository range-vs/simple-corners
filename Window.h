#pragma once

#include <Windows.h>
#include <string>
#include <memory>

#include "Exceptions.h"
#include "Render.h"

class WindowParameters
{
	HINSTANCE hInst;
	int w;
	int h;
public:
	HINSTANCE getHinstance() const noexcept;
	int getWidth() const noexcept;
	int getHeight() const noexcept;
	void setHinstance(HINSTANCE hInst) noexcept;
	void setWidth(int w) noexcept;
	void setHeight(int h) noexcept;
};

class OSWindowsWindow
{
protected:
	HWND hwnd;
	std::wstring classWin;
	HINSTANCE hInst;
	std::unique_ptr<OSWindowsDirect3D11Render> render;

	static LRESULT CALLBACK GlobalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void clear();

public:
	OSWindowsWindow(const OSWindowsWindow&) = delete;
	OSWindowsWindow& operator=(const OSWindowsWindow&) = delete;
	OSWindowsWindow();
	void init(WindowParameters wp);
	void resizeWindow();
	MSG run();
	LRESULT localWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	~OSWindowsWindow();
};