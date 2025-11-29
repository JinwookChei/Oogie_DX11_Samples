#include "stdafx.h"
#include "ParticleSystemCPU.h"
#include "ParticleSystemGPU.h"

#define ResolutionWidth 2560.0f
#define	ResolutionHeigh 1440.0f

#define USE_GPU_PARTICLES 1 // 0: CPU / 1: GPU

#if USE_GPU_PARTICLES
ParticleSystemGPU* g_ParticleGPU = nullptr;
#else
ParticleSystemCPU* g_ParticleCPU = nullptr;
#endif;


LARGE_INTEGER g_Freq;
LARGE_INTEGER g_PrevTime;	


// Init
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr; // Device Context
IDXGISwapChain* g_pSwapChain = nullptr; // 스왑 체인
ID3D11RenderTargetView* g_pRenderTargetView = nullptr; // 렌더 타켓 뷰
ID3D11DepthStencilView* g_pDepthStencilView = nullptr; // 깊이 스텐실 뷰

// ------------------------- Functions ------------------------------------- //
IDXGIAdapter* GetBestAdapter()
{
	IDXGIFactory* pFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
	if (FAILED(hr))
	{
		return nullptr;
	}

	IDXGIAdapter* pBestAdapter = nullptr;
	IDXGIAdapter* pAdapter = nullptr;
	size_t maxDedicatedVedioMemory = 0;

	for (UINT n = 0; pFactory->EnumAdapters(n, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++n)
	{
		DXGI_ADAPTER_DESC desc;
		pAdapter->GetDesc(&desc);

		if (maxDedicatedVedioMemory < desc.DedicatedVideoMemory)
		{
			if (pBestAdapter)
			{
				pBestAdapter->Release();
			}
			pBestAdapter = pAdapter;
			maxDedicatedVedioMemory = desc.DedicatedVideoMemory;
		}
		else
		{
			pAdapter->Release();
		}
	}

	pFactory->Release();
	return pBestAdapter;
}

HRESULT InitDeviceAndSwapChain(HWND hWnd, IDXGIAdapter* pBestAdapter)
{
	// 스왑 체인 구조체를 초기화 해야 함
	DXGI_SWAP_CHAIN_DESC sd;
	memset(&sd, 0x00, sizeof(sd));
	sd.BufferCount = 1; // 백 버퍼의 수
	sd.BufferDesc.Width = ResolutionWidth; // 백 버퍼의 너비
	sd.BufferDesc.Height = ResolutionHeigh; // 백 버퍼의 높이
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 백 버퍼의 포맷
	sd.BufferDesc.RefreshRate.Numerator = 60; // 화면 새로 고침 빈도 ( 분자 )
	sd.BufferDesc.RefreshRate.Denominator = 1; // 화면 새로 고침 빈도 ( 분모 )
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 백 버퍼의 용도
	sd.OutputWindow = hWnd; // 렌더링 할 윈도우 핸들
	sd.SampleDesc.Count = 1; // 멀티샘플링 수
	sd.SampleDesc.Quality = 0; // 멀티샘플링 품질
	sd.Windowed = TRUE; // 창 모드 인지 아닌지.

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		pBestAdapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&g_pSwapChain,
		&g_pd3dDevice,
		nullptr,
		&g_pImmediateContext
	);

	pBestAdapter->Release();

	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	return S_OK;
}

HRESULT InitRenderTargetView()
{
	// 백 버퍼의 렌더 타켓 뷰를 얻어와야한다.
	ID3D11Texture2D* pBackBuffer = nullptr;
	HRESULT hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	return S_OK;
}

HRESULT InitDepthStencilBuffer()
{
	// Depth Stencil Buffer
	D3D11_TEXTURE2D_DESC Desc;
	Desc.Width = ResolutionWidth;
	Desc.Height = ResolutionHeigh;
	Desc.MipLevels = 1;
	Desc.ArraySize = 1;
	Desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Usage = D3D11_USAGE_DEFAULT; // 리소스의 사용법을 지정 함
	/*
		D3D11_USAGE_DEFAULT	= 0, // GPU 에서 주로 사용되며, CPU는 거의 접근하지 않음.
		D3D11_USAGE_IMMUTABLE	= 1, // 생성 후 변경되지 않는 리소스
		D3D11_USAGE_DYNAMIC	= 2,     // CPU에서 자주 업데이트 되며, GPU 에서 읽기 전용으로 사용
		D3D11_USAGE_STAGING	= 3 // CPU와 GPU 간의 데이터 전송에 사용됨
	*/

	Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	Desc.CPUAccessFlags = 0;
	Desc.MiscFlags = 0;

	ID3D11Texture2D* pDepthStencilBuffer = nullptr;
	HRESULT hr = g_pd3dDevice->CreateTexture2D(&Desc, nullptr, &pDepthStencilBuffer);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	hr = g_pd3dDevice->CreateDepthStencilView(pDepthStencilBuffer, nullptr, &g_pDepthStencilView);
	pDepthStencilBuffer->Release();
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	return S_OK;
}

HRESULT SetViewPort()
{
	// 뷰 포트 설정
	D3D11_VIEWPORT vp;
	vp.Width = ResolutionWidth;
	vp.Height = ResolutionHeigh;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	g_pImmediateContext->RSSetViewports(1, &vp);
	return S_OK;
}

void SetRenderTarget()
{
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
}

HRESULT InitD3D(HWND hWnd)
{
	IDXGIAdapter* pBestAdapter = GetBestAdapter();
	if (nullptr == pBestAdapter)
	{
		DEBUG_BREAK();
		return E_FAIL;
	}

	InitDeviceAndSwapChain(hWnd, pBestAdapter);

	InitRenderTargetView();

	InitDepthStencilBuffer();

	SetViewPort();

	SetRenderTarget();
	return S_OK;
}

ID3D11ShaderResourceView* CreateWhiteTextureSRV(ID3D11Device* device)
{
	UINT color = 0xFFFFFFFF;

	D3D11_TEXTURE2D_DESC td = {};
	td.Width = 1;
	td.Height = 1;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = &color;
	initData.SysMemPitch = sizeof(UINT);

	ID3D11Texture2D* tex = nullptr;
	if (FAILED(device->CreateTexture2D(&td, &initData, &tex)))
	{
		return nullptr;
	}

	ID3D11ShaderResourceView* srv = nullptr;
	if (FAILED(device->CreateShaderResourceView(tex, nullptr, &srv)))
	{
		tex->Release();
		return nullptr;
	}
	tex->Release();
	return srv;
}

void BeginPlay()
{
	QueryPerformanceFrequency(&g_Freq);
	QueryPerformanceCounter(&g_PrevTime);


	ID3D11ShaderResourceView* particleTexSRV = CreateWhiteTextureSRV(g_pd3dDevice);
	const int maxParticles = 10000;

#if USE_GPU_PARTICLES
	g_ParticleGPU = new ParticleSystemGPU;
	ParticleSystemGPU::gGpuPatternMode_ = 0;
	if (false == g_ParticleGPU->Initialize(g_pd3dDevice, maxParticles, particleTexSRV))
	{
		if (nullptr != particleTexSRV)
		{
			particleTexSRV->Release();
		}

		DEBUG_BREAK();
		return;
	}
#else
	g_ParticleCPU = new ParticleSystemCPU;
	if (false == g_ParticleCPU->Initialize(g_pd3dDevice, maxParticles, particleTexSRV))
	{
		if (nullptr != particleTexSRV)
		{
			particleTexSRV->Release();
		}

		DEBUG_BREAK();
		return;
	}
#endif;

}

void RenderBegin()
{
	float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Render()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	float dt = float(now.QuadPart - g_PrevTime.QuadPart) / float(g_Freq.QuadPart);
	g_PrevTime = now;
	if (dt > 0.033f)
	{
		dt = 0.033f;
	}

	DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
	DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	float aspect = (float)ResolutionWidth / (float)ResolutionHeigh;
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspect, 0.1f, 1000.0f);
	DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(eye, at, up);
	DirectX::XMMATRIX viewProj = view * proj;

	DirectX::XMFLOAT4X4 viewM;
	DirectX::XMStoreFloat4x4(&viewM, view);
	DirectX::XMFLOAT3 cameraRight(viewM._11, viewM._12, viewM._13);
	DirectX::XMFLOAT3 cameraUp(viewM._21, viewM._22, viewM._23);


#if USE_GPU_PARTICLES
	ParticleSystemGPU::gTimeAcc_ += dt;
	g_ParticleGPU->Update(g_pImmediateContext, dt);
	g_ParticleGPU->Draw(g_pImmediateContext, viewProj, cameraRight, cameraUp);
#else
	g_ParticleCPU->Update(g_pImmediateContext, dt);
	g_ParticleCPU->Draw(g_pImmediateContext, viewProj, cameraRight, cameraUp);
#endif;
	
}

void RenderEnd()
{
	g_pSwapChain->Present(0, 0);
}

void Cleanup()
{

#if USE_GPU_PARTICLES
	if (g_ParticleGPU) delete g_ParticleGPU; g_ParticleGPU = nullptr;
#else
	if (g_ParticleCPU) delete g_ParticleCPU; g_ParticleCPU = nullptr;
#endif;

	if (g_pImmediateContext) g_pImmediateContext->ClearState();
	if (g_pDepthStencilView) g_pDepthStencilView->Release();
	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 윈도우 클래스 등록
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = L"WindowClass";
	RegisterClassEx(&wc);

	// 윈도우 생성
	HWND hWnd = CreateWindowEx(
		0,
		L"WindowClass",
		L"Direct3D 11 윈도우",
		WS_OVERLAPPEDWINDOW,
		100, 100,
		ResolutionWidth, ResolutionHeigh,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	ShowWindow(hWnd, nCmdShow);

	if (FAILED(InitD3D(hWnd)))
	{
		Cleanup();
		return 1;
	}

	BeginPlay();

	MSG msg;
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			RenderBegin();

			Render();

			RenderEnd();
		}
	}

	Cleanup();

	return (int)msg.wParam;
}