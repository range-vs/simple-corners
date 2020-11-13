#include "d3d11_headers.h"

D3D11_BUFFER_DESC Direct3D11CreatorBuffer::createD3D11_BUFFER_DESC(UINT _size, const D3D11_BIND_FLAG& bind_flag, const D3D11_USAGE& usage, int cpuAccessFlag)
{
	D3D11_BUFFER_DESC bd;  // Структура, описывающая создаваемый буфер
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = usage;
	bd.ByteWidth = _size; // размер буфера = размер одной вершины * 3
	bd.BindFlags = bind_flag; // тип буфера - буфер вершин
	bd.CPUAccessFlags = cpuAccessFlag;
	return bd;
}

ComPtr<ID3D11Buffer> Direct3D11CreatorBuffer::createBuffer(const ComPtr<ID3D11Device>& d3dDevice, UINT _size, const D3D11_BIND_FLAG& bind_flag, const D3D11_USAGE& usage, int cpuAccessFlag)
{
	ComPtr<ID3D11Buffer> bufferOut;
	D3D11_BUFFER_DESC bd(createD3D11_BUFFER_DESC(_size, bind_flag, usage, cpuAccessFlag));
	if (FAILED(d3dDevice->CreateBuffer(&bd, NULL, bufferOut.GetAddressOf())))
		throw ExceptionDirect3D11(L"error create buffer!"); 
	return bufferOut;
}


namespace debug
{

	void printError(const wchar_t* er)
	{
		OutputDebugString(er);
	}

}
