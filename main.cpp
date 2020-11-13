#include "Window.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	CoInitialize(nullptr);
	try
	{
		WindowParameters wp;
		wp.setHinstance(hInstance);
		wp.setWidth(1477);
		wp.setHeight(712);

		auto window(std::make_unique<OSWindowsWindow>());
		if (!window)
			throw ExceptionMemory(L"Window is not create, memory not found!\n");
		window->init(wp);
		window->run();
	}
	catch (IException& ex)
	{
		debug::printError(ex.what());
	}
	CoUninitialize();
	return 0;
}
