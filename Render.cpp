#include "Render.h"

#include <array>

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

HWND RenderParameters::getHwnd()const noexcept
{
	return hwnd;
}

int RenderParameters::getClientWidth()const noexcept
{
	return w;
}

int RenderParameters::getClientHeight()const noexcept
{
	return h;
}

void RenderParameters::setHwnd(HWND hwnd)noexcept
{
	this->hwnd = hwnd;
}

void RenderParameters::setClientWidth(int w)noexcept
{
	this->w = w;
}

void RenderParameters::setClientHeight(int h)noexcept
{
	this->h = h;
}

OSWindowsDirect3D11Render::OSWindowsDirect3D11Render():ortho(), vsync(false), running(false), d3dEnable4xMsaa(true), hwnd(nullptr),
desiredColourFormat(),d3dScreenViewport(), d3d4xMsaaQuality()
{

}

void OSWindowsDirect3D11Render::init(RenderParameters rp)
{
	hwnd = rp.getHwnd();

	desiredColourFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D_FEATURE_LEVEL featureLevel;

	// флаги
	UINT createDeviceFlags(0);
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // режим отладки
#endif

	// уровень функций
	std::array<D3D_FEATURE_LEVEL, 8> featureLevels = {
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	// check 4xMSAA
	HRESULT hr(D3D11CreateDeviceAndSwapChain(/*this->adapter*/nullptr, /*D3D_DRIVER_TYPE_UNKNOWN*/D3D_DRIVER_TYPE_HARDWARE,
		nullptr, createDeviceFlags, featureLevels.data(), static_cast<UINT>(featureLevels.size()), D3D11_SDK_VERSION, nullptr,
		nullptr, d3dDevice.GetAddressOf(), &featureLevel, d3dImmediateContext.GetAddressOf()));
	if (FAILED(hr)) 
		throw ExceptionDirect3D11(L"D3D11CreateDevice Failed.\n");

	debug::checkHresult(d3dDevice->CheckMultisampleQualityLevels(desiredColourFormat, 4, &d3d4xMsaaQuality));
	if (d3d4xMsaaQuality <= 0)
		throw ExceptionDirect3D11(L"MSAA quality is not corrected.\n");
	clears::ReleaseCom(d3dDevice);
	clears::ReleaseCom(d3dImmediateContext);

	// создаём цепочку обмена
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = rp.getClientWidth();
	sd.BufferDesc.Height = rp.getClientHeight();
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = desiredColourFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// используем 4xMSAA?
	if (d3dEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = d3d4xMsaaQuality - 1;
	}
	// не используем
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hwnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = D3D11CreateDeviceAndSwapChain(/*this->adapter*/nullptr, D3D_DRIVER_TYPE_HARDWARE,
		nullptr, createDeviceFlags, featureLevels.data(), static_cast<UINT>(featureLevels.size()), D3D11_SDK_VERSION, &sd,
		d3dSwapChain.GetAddressOf(), d3dDevice.GetAddressOf(), &featureLevel, d3dImmediateContext.GetAddressOf());
	if (FAILED(hr)) // успешно ли создали?
		throw ExceptionDirect3D11(L"D3D11CreateDevice Failed.\n");
	if (featureLevel < D3D_FEATURE_LEVEL_11_0) // проверка доступности d3d11 на локальном ПК
		throw ExceptionDirect3D11(L"Direct3D Feature Level 11 unsupported.\n");

	// состояние теста глубины
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	hr = d3dDevice->CreateDepthStencilState(&depthStencilDesc, d3dDepthStencilEnableDepth.GetAddressOf());
	if (FAILED(hr))
		throw ExceptionDirect3D11(L"Direct3D error create d3dDepthStencilEnableDepth");

	depthStencilDesc.DepthEnable = false;
	hr = d3dDevice->CreateDepthStencilState(&depthStencilDesc, d3dDepthStencilDisableDepth.GetAddressOf());
	if (FAILED(hr))
		throw ExceptionDirect3D11(L"Direct3D Feature Level 11 unsupported.");

	d3dImmediateContext->OMSetDepthStencilState(d3dDepthStencilDisableDepth.Get(), 1); // сразу отрубили для 2D

	// disable alt + enter
	ComPtr<IDXGIDevice> pDXGIDevice;
	hr = d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)pDXGIDevice.GetAddressOf());
	if (FAILED(hr))
		throw ExceptionDirect3D11(L"IDXGIDevice Failed.");

	ComPtr<IDXGIAdapter> pDXGIAdapter;
	hr = pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)pDXGIAdapter.GetAddressOf());
	if (FAILED(hr))
		throw ExceptionDirect3D11(L"IDXGIAdapter Failed.");

	ComPtr<IDXGIFactory> pIDXGIFactory;
	hr = pDXGIAdapter->GetParent(__uuidof(IDXGIFactory), (void**)pIDXGIFactory.GetAddressOf());
	if (FAILED(hr))
		throw ExceptionDirect3D11(L"IDXGIFactory Failed.");

	hr = pIDXGIFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(hr))
		throw ExceptionDirect3D11(L"MakeWindowAssociation Failed.");

	// first draw
	resizeViewport(rp.getClientWidth(), rp.getClientHeight());

	scene = std::make_unique<Direct3D11Scene>(d3dDevice, d3dImmediateContext);
	if (!scene)
		throw ExceptionMemory(L"Scene is not create, memory not found!\n");
	SceneParameters sp;
	RECT rect;
	if (!GetClientRect(hwnd, &rect))
		throw new ExceptionWindow(L"Error call GetClientRect.\n");
	sp.setClientWidth(rect.right - rect.left);
	sp.setClientHeight(rect.bottom - rect.top);
	scene->init(sp);
	scene->resizeViewport(sp.getClientWidth(), sp.getClientHeight());

	w = sp.getClientWidth();
	h = sp.getClientHeight();

	running = true;
}

void OSWindowsDirect3D11Render::resizeViewport(int w, int h)
{
	// проверка, что буферы существуют
	if (!d3dImmediateContext || !d3dDevice || !d3dSwapChain)
		throw ExceptionDirect3D11(L"Error resize D3D11 render.");

	// теперь удаляем буферы
	clears::ReleaseCom(d3dRenderTargetView);
	clears::ReleaseCom(d3dDepthStencilView);
	clears::ReleaseCom(d3dDepthStencilBuffer);

	// выполняем ресайз заднего буфера
	debug::checkHresult(d3dSwapChain->ResizeBuffers(1, w, h, desiredColourFormat, 0));
	ComPtr<ID3D11Texture2D> backBuffer;
	debug::checkHresult(d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf())));
	debug::checkHresult(d3dDevice->CreateRenderTargetView(backBuffer.Get(), 0, d3dRenderTargetView.GetAddressOf()));
	clears::ReleaseCom(backBuffer);

	// настраиваем рендер по новой
	// буфер глубины / трафарета и вида
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = w;
	depthStencilDesc.Height = h;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// используем 4xMSAA?
	if (d3dEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = d3d4xMsaaQuality - 1;
	}
	// не используем
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	debug::checkHresult(d3dDevice->CreateTexture2D(&depthStencilDesc, 0, d3dDepthStencilBuffer.GetAddressOf()));
	debug::checkHresult(d3dDevice->CreateDepthStencilView(d3dDepthStencilBuffer.Get(), 0, d3dDepthStencilView.GetAddressOf()));

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	// устанавливаем буфер глубины / трафарета в рендеринг
	d3dImmediateContext->OMSetRenderTargets(1, d3dRenderTargetView.GetAddressOf(), d3dDepthStencilView.Get()); // увеличение памяти с 20 мб до 100 мб (???)
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));

	// устанавливаем пространство вида
	d3dScreenViewport.TopLeftX = 0;
	d3dScreenViewport.TopLeftY = 0;
	d3dScreenViewport.Width = static_cast<float>(w);
	d3dScreenViewport.Height = static_cast<float>(h);
	d3dScreenViewport.MinDepth = 0.0f;
	d3dScreenViewport.MaxDepth = 1.0f;

	d3dImmediateContext->RSSetViewports(1, &d3dScreenViewport);

	ortho = DirectX::XMMatrixOrthographicLH(static_cast<float>(w), static_cast<float>(h), 0.0f, 1000.0f);
	if(scene)
		scene->resizeViewport(w, h);

	this->w = w;
	this->h = h;
}

void OSWindowsDirect3D11Render::clear()
{

}

void OSWindowsDirect3D11Render::render()
{
	scene->render(d3dRenderTargetView, d3dDepthStencilView, ortho);
	debug::checkHresult(d3dSwapChain->Present(vsync ? 1 : 0, 0));
}

void OSWindowsDirect3D11Render::update()
{
	scene->update(w, h);
}

bool OSWindowsDirect3D11Render::isRunning() const noexcept
{
	return running;
}

void OSWindowsDirect3D11Render::setRunning(bool r)noexcept
{
	running = r;
}

const std::unique_ptr<Direct3D11Scene>& OSWindowsDirect3D11Render::getScene() const
{
	return scene;
}

OSWindowsDirect3D11Render::~OSWindowsDirect3D11Render()
{
	clear();
}