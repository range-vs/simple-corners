#include "Window.h"

#include "windowsx.h"

HINSTANCE WindowParameters::getHinstance()const noexcept
{
	return hInst;
}

int WindowParameters::getWidth()const noexcept
{
	return w;
}

int WindowParameters::getHeight()const noexcept
{
	return h;
}

void WindowParameters::setHinstance(HINSTANCE hInst) noexcept
{
	this->hInst = hInst;
}

void WindowParameters::setWidth(int w) noexcept
{
	this->w = w;
}

void WindowParameters::setHeight(int h) noexcept
{
	this->h = h;
}

LRESULT OSWindowsWindow::GlobalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto wndptr(reinterpret_cast<OSWindowsWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA))); // (OSWindowsWindow*)
	if (!wndptr)
		return DefWindowProc(hwnd, msg, wParam, lParam);
	return wndptr->localWndProc(hwnd, msg, wParam, lParam);
}

OSWindowsWindow::OSWindowsWindow():hwnd(nullptr), hInst(nullptr)
{
}

void OSWindowsWindow::init(WindowParameters wp)
{
	classWin = L"oswindows_class_window";
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = GlobalWndProc;
	wc.hInstance = wp.getHinstance();
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	// wc.lpszMenuName = NULL;
	wc.lpszClassName = classWin.c_str();
	if (!RegisterClassEx(&wc))
		throw ExceptionWindow(L"Error register window\n");
	hwnd = CreateWindowEx(0,
		classWin.c_str(),
		L"Corners",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wp.getWidth(), wp.getHeight(),
		nullptr,
		nullptr,
		wp.getHinstance(),
		nullptr);
	if (!hwnd)
		throw ExceptionWindow(L"Error create window\n");

	ShowWindow(hwnd, SW_SHOW);

	render = std::make_unique<OSWindowsDirect3D11Render>();
	if (!render)
		throw ExceptionMemory(L"Render is not create, memory not found!\n");
	RenderParameters rp;
	rp.setHwnd(hwnd);
	RECT rect;
	if (!GetWindowRect(hwnd, &rect))
		throw ExceptionWindow(L"Error call GetWindowRect.\n");
	rp.setClientWidth(rect.right - rect.left);
	rp.setClientHeight(rect.bottom - rect.top);
	render->init(rp);
}

void OSWindowsWindow::resizeWindow()
{
	WINDOWPLACEMENT winPlace;
	winPlace.length = sizeof(WINDOWPLACEMENT);
	if(!GetWindowPlacement(hwnd, &winPlace)) 
		throw ExceptionWindow(L"Error call GetWindowPlacement.\n");
	if (winPlace.showCmd == SW_HIDE || winPlace.showCmd == SW_SHOWMINIMIZED) // если окно свернуто
	{
		render->setRunning(false); // остановка рендеринга
		return;
	}
	else if (winPlace.showCmd == SW_RESTORE || winPlace.showCmd == SW_SHOWNORMAL) // если окно восстановлено
		render->setRunning(true); // продолжаем рендеринг

	if (render->isRunning())
	{
		RECT rect;
		if (!GetClientRect(hwnd, &rect))
			throw ExceptionWindow(L"Error call GetClientRect.\n");
		render->resizeViewport(rect.right - rect.left, rect.bottom - rect.top);
	}
}

MSG OSWindowsWindow::run()
{
	MSG msg;
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(this)); // передаем указатель на окно
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (render->isRunning())
			{
				render->update();
				render->render();
			}
			else
				Sleep(100);
		}
	}
	return msg;
}

void OSWindowsWindow::clear()
{
	UnregisterClass(classWin.c_str(), hInst);
}

OSWindowsWindow::~OSWindowsWindow()
{
	clear();
}

LRESULT OSWindowsWindow::localWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
	{
		resizeWindow();
		break;
	}

	case WM_MOUSEMOVE:
	{
		auto xPos(GET_X_LPARAM(lParam));
		auto yPos(GET_Y_LPARAM(lParam));
		RECT rect;
		if (!GetClientRect(hwnd, &rect))
			throw ExceptionWindow(L"Error call GetClientRect.\n");
		render->getScene()->checkMouseHoverChess((rect.right - rect.left), (rect.bottom - rect.top), xPos, yPos);
		break;
	}

	case WM_LBUTTONDOWN:
	{
		auto xPos(GET_X_LPARAM(lParam));
		auto yPos(GET_Y_LPARAM(lParam));
		RECT rect;
		if (!GetClientRect(hwnd, &rect))
			throw ExceptionWindow(L"Error call GetClientRect.\n");
		render->getScene()->checkMouseClickChess((rect.right - rect.left), (rect.bottom - rect.top), xPos, yPos);
		break;
	}

	case WM_ERASEBKGND:
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
