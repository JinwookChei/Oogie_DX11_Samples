#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <ctime>

#include "ParticleSystemCPU.h"
#include "ParticleSystemGPU.h"

#define USE_GPU_PARTICLES 0 // 0: CPU / 1: GPU

#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler")

HWND g_hWnd = nullptr;
int g_ClientWidth = 1280;
int g_ClientHeight = 720;

bool g_Running = true;

ID3D11Device* g_Device = nullptr;
ID3D11DeviceContext* g_Context = nullptr;
IDXGISwapChain* g_SwapChain = nullptr;
ID3D11RenderTargetView* g_RTV = nullptr;

LARGE_INTEGER gFreq_;
LARGE_INTEGER gprevTime_;

#if USE_GPU_PARTICLES
ParticleSystemGPU gParticleGPU;
#else
ParticleSystemCPU gParticleCPU;
#endif;

float gPitch = 0.0f;
float gYaw = 0.0f;

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

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		g_Running = false;
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(hWnd);
			return 0;
		}
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = TEXT("DX11ParticleWindowClass");

	if (!RegisterClassEx(&wc))
		return false;

	RECT rc = { 0, 0, g_ClientWidth, g_ClientHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	g_hWnd = CreateWindow(
		wc.lpszClassName,
		TEXT("DirectX11 Particle Demo (CPU/GPU + Fountain Mode)"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr, nullptr,
		hInstance, nullptr);

	if (!g_hWnd)
		return false;

	ShowWindow(g_hWnd, nCmdShow);
	return true;
}

bool InitD3D()
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2; // flip-model은 2 이상 권장
	sd.BufferDesc.Width = g_ClientWidth;
	sd.BufferDesc.Height = g_ClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = 0;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};

	D3D_FEATURE_LEVEL featureLevel;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		featureLevels,
		2,
		D3D11_SDK_VERSION,
		&sd,
		&g_SwapChain,
		&g_Device,
		&featureLevel,
		&g_Context
	);

	if (FAILED(hr))
		return false;

	ID3D11Texture2D* backBuffer = nullptr;
	hr = g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (FAILED(hr))
		return false;

	hr = g_Device->CreateRenderTargetView(backBuffer, nullptr, &g_RTV);
	backBuffer->Release();
	if (FAILED(hr))
		return false;

	g_Context->OMSetRenderTargets(1, &g_RTV, nullptr);

	D3D11_VIEWPORT vp = {};
	vp.Width = (FLOAT)g_ClientWidth;
	vp.Height = (FLOAT)g_ClientHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;

	g_Context->RSSetViewports(1, &vp);

	return true;
}

void Cleanup()
{
	if (g_Context) g_Context->ClearState();

	if (g_RTV) g_RTV->Release();
	if (g_SwapChain) g_SwapChain->Release();
	if (g_Context) g_Context->Release();
	if (g_Device) g_Device->Release();
}

void Render()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	float dt = float(now.QuadPart - gprevTime_.QuadPart) / float(gFreq_.QuadPart);
	gprevTime_ = now;
	if (dt > 0.033f)
	{
		dt = 0.033f;
	}

#if USE_GPU_PARTICLES
	ParticleSystemGPU::gTimeAcc_ += dt;

	ParticleSystemGPU::gGpuPatternMode_ = 0;
#endif;

	DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
	DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	float aspect = (float)g_ClientWidth / (float)g_ClientHeight;
	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspect, 0.1f, 1000.0f);

	/*DirectX::XMVECTOR lookDir = DirectX::XMVectorSet(cosf(gPitch) * sinf(gYaw), sinf(gPitch), cosf(gPitch) * cosf(gYaw), 0.0f);
	DirectX::XMVECTOR newAt = DirectX::XMVectorAdd(eye, lookDir);*/

	DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(eye, at, up);
	DirectX::XMMATRIX viewProj = view * proj;

	DirectX::XMFLOAT4X4 viewM;
	DirectX::XMStoreFloat4x4(&viewM, view);
	DirectX::XMFLOAT3 cameraRight(viewM._11, viewM._12, viewM._13);
	DirectX::XMFLOAT3 cameraUp(viewM._21, viewM._22, viewM._23);

#if USE_GPU_PARTICLES
	gParticleGPU.Update(g_Context, dt);
#else
	gParticleCPU.Update(g_Context, dt);
#endif;

	float clearColor[4] = { 0.1f, 0.1f, 0.15f, 1.0f };
	g_Context->OMSetRenderTargets(1, &g_RTV, nullptr);
	g_Context->ClearRenderTargetView(g_RTV, clearColor);

#if USE_GPU_PARTICLES
	gParticleGPU.Draw(g_Context, viewProj, cameraRight, cameraUp);
#else
	gParticleCPU.Draw(g_Context, viewProj, cameraRight, cameraUp);
#endif;

	g_SwapChain->Present(1, 0);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	srand((unsigned int)time(nullptr));

	if (!InitWindow(hInstance, nCmdShow))
	{
		return 0;
	}

	if (!InitD3D())
	{
		return 0;
	}

	QueryPerformanceFrequency(&gFreq_);
	QueryPerformanceCounter(&gprevTime_);

	ID3D11ShaderResourceView* particleTexSRV = CreateWhiteTextureSRV(g_Device);

	const int maxParticles = 10000;

#if USE_GPU_PARTICLES
	if (false == gParticleGPU.Initialize(g_Device, maxParticles, particleTexSRV))
	{
		if (nullptr != particleTexSRV)
		{
			particleTexSRV->Release();
		}
		return 0;
	}
#else
	if (false == gParticleCPU.Initialize(g_Device, maxParticles, particleTexSRV))
	{
		if (nullptr != particleTexSRV)
		{
			particleTexSRV->Release();
		}
		return 0;
	}
#endif;

	MSG msg = {};
	while (g_Running)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				g_Running = false;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (false == g_Running)
		{
			break;
		}

		Render();
	}

	if (nullptr != particleTexSRV)
	{
		particleTexSRV->Release();
	}
	Cleanup();
	return (int)msg.wParam;
}