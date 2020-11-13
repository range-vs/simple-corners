#pragma once

#include "Scene.h"

class RenderParameters
{
	HWND hwnd;
	int w;
	int h;

public:
	HWND getHwnd()const noexcept;
	int getClientWidth()const noexcept;
	int getClientHeight()const noexcept;
	void setHwnd(HWND hInst)noexcept;
	void setClientWidth(int w)noexcept;
	void setClientHeight(int h)noexcept;
};

class OSWindowsDirect3D11Render
{
protected:
	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<ID3D11DeviceContext> d3dImmediateContext;
	ComPtr<IDXGISwapChain> d3dSwapChain;

	ComPtr<ID3D11Texture2D> d3dDepthStencilBuffer;
	ComPtr<ID3D11DepthStencilState> d3dDepthStencilEnableDepth;
	ComPtr<ID3D11DepthStencilState> d3dDepthStencilDisableDepth;
	ComPtr<ID3D11RenderTargetView> d3dRenderTargetView;
	ComPtr<ID3D11DepthStencilView> d3dDepthStencilView;
	D3D11_VIEWPORT d3dScreenViewport;

	UINT d3d4xMsaaQuality;
	bool d3dEnable4xMsaa;
	bool running;
	bool vsync;

	HWND hwnd;
	DXGI_FORMAT desiredColourFormat;
	DirectX::XMMATRIX ortho;

	std::unique_ptr<Direct3D11Scene> scene;

	int w;
	int h;

public:
	OSWindowsDirect3D11Render(const OSWindowsDirect3D11Render&) = delete;
	OSWindowsDirect3D11Render& operator=(const OSWindowsDirect3D11Render&) = delete;
	OSWindowsDirect3D11Render();
	void init(RenderParameters wp);
	void resizeViewport(int w, int h);
	void clear();
	void render();
	void update();
	bool isRunning()const noexcept;
	void setRunning(bool r)noexcept;
	const std::unique_ptr<Direct3D11Scene>& getScene() const;

	~OSWindowsDirect3D11Render() ;
};

