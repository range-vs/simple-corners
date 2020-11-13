#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include "dxerr.h"
#include <DirectXMath.h>

#include "DDSTextureLoader11.h"

#include <system_error>

#include <atlbase.h>
#include <wrl.h>

#include <memory>

#include "Exceptions.h"

using namespace Microsoft::WRL;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment( lib, "dxguid.lib")

class Direct3D11CreatorBuffer
{
	static D3D11_BUFFER_DESC createD3D11_BUFFER_DESC(UINT _size, const D3D11_BIND_FLAG& bind_flag, const D3D11_USAGE& usage, int cpuAccessFlag);
public:
	template <class Data>
	static ComPtr<ID3D11Buffer> createBuffer(const ComPtr<ID3D11Device>& d3dDevice, UINT _size, const D3D11_BIND_FLAG& bind_flag, const D3D11_USAGE& usage, int cpuAccessFlag, Data* data)
	{
		ComPtr<ID3D11Buffer> bufferOut;
		D3D11_BUFFER_DESC bd(createD3D11_BUFFER_DESC(_size, bind_flag, usage, cpuAccessFlag));
		D3D11_SUBRESOURCE_DATA InitData; // Структура, содержащая данные буфера
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = data;
		if (FAILED(d3dDevice->CreateBuffer(&bd, &InitData, bufferOut.GetAddressOf())))
			throw ExceptionDirect3D11(L"error create buffer!\n"); // написать другой класс
		return bufferOut;
	}

	static ComPtr<ID3D11Buffer> createBuffer(const ComPtr<ID3D11Device>& d3dDevice, UINT _size, const D3D11_BIND_FLAG& bind_flag, const D3D11_USAGE& usage, int cpuAccessFlag);
};


namespace clears
{
	template<class T>
	void ReleaseCom(T& com)
	{
		if (com)
			com.Reset();
		com = nullptr;
	}
}

namespace debug
{
	void printError(const wchar_t* er);

#if defined(DEBUG) | defined(_DEBUG)
	template<class T>
	void checkHresult(T val)
	{
		HRESULT hr(val);
		if (FAILED(hr))
		{
			const char* line(__FILE__);
			std::wstring wline(line, line + strlen(line));
			std::string err(std::system_category().message(hr));
			std::wstring werr(err.begin(), err.end());
			DXTrace(wline.c_str(), (DWORD)__LINE__, hr, werr.c_str(), true);
			//throw ExceptionDirect3D11(L"Error in checkHresult");
		}
	}
#else
	template<class T>
	bool checkHresult(T val)
	{
		return true;
	}
#endif 
}
