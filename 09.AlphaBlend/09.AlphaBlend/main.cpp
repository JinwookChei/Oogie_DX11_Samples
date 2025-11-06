#include "stdafx.h"

#define ResolutionWidth 2560.0f
#define	ResolutionHeigh 1440.0f
//#define ResolutionWidth 1280.0f
//#define	ResolutionHeigh 720.0f
//#define ResolutionWidth 800.0f
//#define	ResolutionHeigh 600.0f

// ADD
// ID3D11BlendState* g_pAlphaBlendState = nullptr;
// HRESULT InitAlphaBlendState()
// OMSetting

// Init
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr; // Device Context
IDXGISwapChain* g_pSwapChain = nullptr; // 스왑 체인
ID3D11RenderTargetView* g_pRenderTargetView = nullptr; // 렌더 타켓 뷰
ID3D11DepthStencilView* g_pDepthStencilView = nullptr; // 깊이 스텐실 뷰

// Render
ID3D11InputLayout* g_pInputLayout = nullptr;
ID3D11Buffer* g_pVertexBuffer = nullptr;
ID3D11Buffer* g_pIndexBuffer = nullptr;
ID3D11Buffer* g_pConstantBuffer;
ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;

// Texture
//ID3D11Texture2D* g_pTextureResource = nullptr;
ID3D11ShaderResourceView* g_pTextureResourceView = nullptr;
ID3D11SamplerState* g_pSamplerLinear = nullptr;

// Normal Mapping
ID3D11ShaderResourceView* g_pNormalMapShaderResourceView = nullptr;

// Alpha Blend
// **Chage**
ID3D11BlendState* g_pAlphaBlendState = nullptr;

struct SimpleVertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT4 tangent;
};

struct ConstantBuffer
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;

	//Light
	// 4Byte 단위로 Padding 줘야함.
	DirectX::XMFLOAT4 lightColor;
	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT3 spotPosition;
	float spotRange;
	DirectX::XMFLOAT3 spotDirection;
	float spotAngle;

};

SimpleVertex vertices[] = {
	// 아래 (-Z)
	{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT3(0,0,-1), DirectX::XMFLOAT2(0,1), DirectX::XMFLOAT4(-1,0,0,1)},
	{DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT3(0,0,-1), DirectX::XMFLOAT2(0,0), DirectX::XMFLOAT4(-1,0,0,1)},
	{DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT3(0,0,-1), DirectX::XMFLOAT2(1,0), DirectX::XMFLOAT4(-1,0,0,1)},
	{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT3(0,0,-1), DirectX::XMFLOAT2(1,1), DirectX::XMFLOAT4(-1,0,0,1)},
	// 위 (+Z)
	{DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT3(0,0,1), DirectX::XMFLOAT2(1,1),DirectX::XMFLOAT4(1,0,0,1)},
	{DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT3(0,0,1), DirectX::XMFLOAT2(0,1),DirectX::XMFLOAT4(1,0,0,1)},
	{DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT3(0,0,1), DirectX::XMFLOAT2(0,0),DirectX::XMFLOAT4(1,0,0,1)},
	{DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT3(0,0,1), DirectX::XMFLOAT2(1,0),DirectX::XMFLOAT4(1,0,0,1)},
	// 앞 (-X)
	{DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT3(-1,0,0), DirectX::XMFLOAT2(0,1),DirectX::XMFLOAT4(0,0,1,1)},
	{DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT3(-1,0,0), DirectX::XMFLOAT2(0,0),DirectX::XMFLOAT4(0,0,1,1)},
	{DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT3(-1,0,0), DirectX::XMFLOAT2(1,0),DirectX::XMFLOAT4(0,0,1,1)},
	{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT3(-1,0,0), DirectX::XMFLOAT2(1,1),DirectX::XMFLOAT4(0,0,1,1)},
	// 뒤 (+X)
	{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT3(1,0,0), DirectX::XMFLOAT2(0,1),DirectX::XMFLOAT4(0,0,-1,1)},
	{DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT3(1,0,0), DirectX::XMFLOAT2(0,0),DirectX::XMFLOAT4(0,0,-1,1)},
	{DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT3(1,0,0), DirectX::XMFLOAT2(1,0),DirectX::XMFLOAT4(0,0,-1,1)},
	{DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT3(1,0,0), DirectX::XMFLOAT2(1,1),DirectX::XMFLOAT4(0,0,-1,1)},
	// 오른쪽 (+Y)
	{DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT3(0,1,0), DirectX::XMFLOAT2(0,1),DirectX::XMFLOAT4(1,0,0,1)},
	{DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT3(0,1,0), DirectX::XMFLOAT2(0,0),DirectX::XMFLOAT4(1,0,0,1)},
	{DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT3(0,1,0), DirectX::XMFLOAT2(1,0),DirectX::XMFLOAT4(1,0,0,1)},
	{DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT3(0,1,0), DirectX::XMFLOAT2(1,1),DirectX::XMFLOAT4(1,0,0,1)},
	// 왼쪽 (-Y)
	{DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT3(0,-1,0), DirectX::XMFLOAT2(1,1),DirectX::XMFLOAT4(-1,0,1,1)},
	{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT3(0,-1,0), DirectX::XMFLOAT2(0,1),DirectX::XMFLOAT4(-1,0,1,1)},
	{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT3(0,-1,0), DirectX::XMFLOAT2(0,0),DirectX::XMFLOAT4(-1,0,1,1)},
	{DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT3(0,-1,0), DirectX::XMFLOAT2(1,0),DirectX::XMFLOAT4(-1,0,1,1)},
};

WORD indices[] = {
	0,1,2, 0,2,3,       // Front
	4,5,6, 4,6,7,       // Back
	8,9,10, 8,10,11,    // Left
	12,13,14, 12,14,15, // Right
	16,17,18, 16,18,19, // Top
	20,21,22, 20,22,23  // Bottom
};

// Misc
float g_fRotaionAngle = 0.0f;

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

	return S_OK;
}

HRESULT InitVertexBuffer()
{
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0x00, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(vertices);
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	memset(&InitData, 0x00, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = vertices;

	HRESULT hr = g_pd3dDevice->CreateBuffer(&desc, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}
	return S_OK;
}

HRESULT InitIndexBuffer()
{
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0x00, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(indices);
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	memset(&InitData, 0x00, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = indices;

	HRESULT hr = g_pd3dDevice->CreateBuffer(&desc, &InitData, &g_pIndexBuffer);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}
	return S_OK;
}

HRESULT InitConstantBuffer()
{
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0x00, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(ConstantBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;

	HRESULT hr = g_pd3dDevice->CreateBuffer(&desc, nullptr, &g_pConstantBuffer);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	return S_OK;
}

HRESULT InitInputLayout(ID3DBlob* pVSBlob)
{
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	HRESULT hr = g_pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pInputLayout);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	return S_OK;
}

HRESULT InitVertexShader()
{
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = D3DReadFileToBlob(L"VertexShader.cso", &pVSBlob);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		pVSBlob->Release();
		return hr;
	}

	InitInputLayout(pVSBlob);

	pVSBlob->Release();
	return S_OK;
}

HRESULT InitPixelShader()
{
	ID3DBlob* pPSBlob = nullptr;
	HRESULT hr = D3DReadFileToBlob(L"PixelShader.cso", &pPSBlob);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	return S_OK;
}

HRESULT LoadTextureWithDirectXTex(ID3D11Device* device, const wchar_t* fileName, bool isNormalMap, ID3D11ShaderResourceView** outSRV)
{
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage scratchImg;
	HRESULT hr = DirectX::LoadFromWICFile(fileName, DirectX::WIC_FLAGS_NONE, &metadata, scratchImg);
	if (FAILED(hr))
	{
		hr = DirectX::LoadFromDDSFile(fileName, DirectX::DDS_FLAGS_NONE, &metadata, scratchImg);
		if (FAILED(hr))
		{
			DEBUG_BREAK();
			return hr;
		}
	}
	const DirectX::Image* pImg = scratchImg.GetImage(0, 0, 0);

	DirectX::ScratchImage convImg;
	if (isNormalMap)
	{
		// Normal Map → UNORM (Linear)
		if (metadata.format != DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			hr = DirectX::Convert(
				pImg, 1, metadata,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				DirectX::TEX_FILTER_DEFAULT,
				DirectX::TEX_THRESHOLD_DEFAULT,
				convImg
			);
			if (FAILED(hr)) return hr;

			pImg = convImg.GetImage(0, 0, 0);
			metadata.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
	}
	else
	{
		if (metadata.format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
		{
			hr = DirectX::Convert(
				pImg, 1, metadata,
				DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
				DirectX::TEX_FILTER_DEFAULT,
				DirectX::TEX_THRESHOLD_DEFAULT,
				convImg
			);
			if (FAILED(hr)) return hr;

			pImg = convImg.GetImage(0, 0, 0);
			metadata.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		}
	}

	ID3D11Texture2D* pTexture = nullptr;
	hr = DirectX::CreateTexture(device, pImg, 1, metadata, (ID3D11Resource**)&pTexture);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = (UINT)metadata.mipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(pTexture, &srvDesc, outSRV);
	pTexture->Release();
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	return S_OK;
}

HRESULT InitTexture()
{
	//const wchar_t* textureFile = L"../../Resource/Bricks_2K/Bricks_Color.png";
	//const wchar_t* textureFile = L"../../Resource/Bricks_4K/Bricks_Color.png";
	//const wchar_t* textureFile = L"../../Resource/Stones_2K/Stones_Color.png";
	//const wchar_t* textureFile = L"../../Resource/Stones_4K/Stones_Color.png";
	const wchar_t* textureFile = L"../../Resource/Ragnarok_Online_Acolyte.png";
	//const wchar_t* textureFile = L"../../Resource/BrickTexture.jpg";
	HRESULT hr = LoadTextureWithDirectXTex(g_pd3dDevice, textureFile, false, &g_pTextureResourceView);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}
	
	//const wchar_t* normalFile = L"../../Resource/Bricks_2K/Bricks_NormalDX.png";
	//const wchar_t* normalFile = L"../../Resource/Bricks_4K/Bricks_NormalDX.png";
	//const wchar_t* normalFile = L"../../Resource/Stones_2K/Stones_NormalDX.png";
	//const wchar_t* normalFile = L"../../Resource/Stones_4K/Stones_NormalDX.png";
	//const wchar_t* normalFile = L"../../Resource/BrickNormal.jpg";
	/*hr = LoadTextureWithDirectXTex(g_pd3dDevice, normalFile, true, &g_pNormalMapShaderResourceView);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}*/



	D3D11_SAMPLER_DESC samplerDesc = {};
	// Point 샘플링
	// samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;	// 0 ~ 1 최근접  -> 계단형태로 보임 3D
	// Linear 샘플링
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// 0 ~ 1 선형 U V  -> 주위 4개의 픽셀 
	// Anisotropic 샘플링
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;         // 선명하게 보여야할때 -> 16개
	samplerDesc.MaxAnisotropy = 8;

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // 반복
	//samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP; // 경계 고정
	//samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER; // 
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // 반복
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; // 반복

	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = g_pd3dDevice->CreateSamplerState(&samplerDesc, &g_pSamplerLinear);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	return hr;
}

// **Change""
HRESULT InitAlphaBlendState()
{
	//D3D11_BLEND_DESC blendDesc = {};
	// 설명:
	// 멀티샘플링(MSAA) 이 켜져 있을 때, 알파 채널을 사용해 픽셀 커버리지를 제어하는 기능입니다.
	// 즉, 알파값(투명도)을 이용해 멀티샘플링 픽셀을 부분적으로 활성화시킬 수 있습니다.
	// 주 사용처:
	// 반투명 객체(예: 잎사귀 텍스처)를 알파 테스트 없이 부드럽게 표현하고 싶을 때
	// 멀티샘플 안티에일리어싱(MSAA) 활성화 시에만 의미가 있습니다.
	//blendDesc.AlphaToCoverageEnable;

	// 설명:
	// 렌더 타겟이 여러 개 있을 때(MRT, Multiple Render Targets)
	// 각 렌더 타겟마다 다른 블렌딩 설정을 사용할 수 있게 해줍니다.
	// 기본 동작 :
	// FALSE이면 RenderTarget[0]의 블렌드 설정이 모든 렌더 타겟에 동일하게 적용됩니다.
	// TRUE이면 각 RenderTarget[i]마다 개별적인 설정(BlendEnable, SrcBlend, DestBlend 등)을 지정할 수 있습니다.
	//blendDesc.IndependentBlendEnable;

	// 설명:
	// 각 렌더 타겟(MRT)에 대한 블렌딩 설정을 저장하는 배열입니다.
	// 최대 8개의 렌더 타겟에 대해 각각 다음을 설정할 수 있습니다 :
	// 기본 공식:
	// 최종색 = (소스색 * 소스알파) + (배경색 * (1 - 소스알파))
	//blendDesc.RenderTarget[0].BlendEnable = TRUE;
	//blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;		// 소스 알파값 지금 그려질 오브젝트
	//blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;	//// 1 - 소스 알파값  // 렌더타켓

	//blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	//blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

	//blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	//blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	//
	//blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	//HRESULT hr = g_pd3dDevice->CreateBlendState(&blendDesc, &g_pAlphaBlendState);
	//if (FAILED(hr))
	//{
	//	DEBUG_BREAK();
	//	return hr;
	//}
	//return hr;

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // 소스 알파값 지금 그려질 오브젝트
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 1 - 소스 알파값  // 렌더타켓
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	// BlendFactor : 색을 강조하거나, 화면 전체 페이드 같은 특수 효과를 만들 때 사용. ->OMSetBlendState()에서 BlendFactor 값 인자로 넘김.
	//blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
	//blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR; // 1 - BlendFactor  // 렌더타켓

	
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT hr = g_pd3dDevice->CreateBlendState(&blendDesc, &g_pAlphaBlendState);
	return hr;
}

void UpdateConstantResource(const Transform& worldTransform)
{
	Float4 worldPosition = worldTransform.GetPosition();
	DirectX::XMMATRIX scale = worldTransform.GetScaleMatrix();
	DirectX::XMMATRIX rotation = worldTransform.GetRotationMatrix();
	DirectX::XMMATRIX position = worldTransform.GetPositionMatrix();

	// 월드, 뷰, 프로젝션 행렬 설정
	// world는 오브젝트마다 고유의 값이며, 각각의 오브젝트의 Transform 을 적용해야함.
	DirectX::XMMATRIX world = scale * rotation * position;
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(-5.0f, -1.0f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, ResolutionWidth / ResolutionHeigh, 0.01f, 100.0f);

	// Spot Light
	DirectX::XMFLOAT4 lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT4 ambientColor = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.3f);
	DirectX::XMFLOAT3 spotPosition = DirectX::XMFLOAT3(-2.0f, -1.0f, 0.0f);
	DirectX::XMFLOAT3 spotDirection = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	float spotRange = 20.0f;
	float spotAngle = cosf(DirectX::XMConvertToRadians(20.0f));

	// 상수 버퍼 업데이트
	ConstantBuffer cb;
	cb.world = DirectX::XMMatrixTranspose(world);
	cb.view = DirectX::XMMatrixTranspose(view);
	cb.projection = DirectX::XMMatrixTranspose(projection);
	cb.lightColor = lightColor;
	cb.ambientColor = ambientColor;
	cb.spotPosition = spotPosition;
	cb.spotDirection = spotDirection;
	cb.spotRange = spotRange;
	cb.spotAngle = spotAngle;

	g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb, 0, 0);
}

void IASetting()
{
	g_pImmediateContext->IASetInputLayout(g_pInputLayout);
	// 버텍스 버퍼 설정
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	// 인덱스 버퍼 설정
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// 프리미티브 유형 설정
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void VSSetting()
{
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);

	// StartSlot : Buffer Slot
	// NumBuffers : Buffer가 2개 이상인 배열인 경우 설정.
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
}

void RSSetting()
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
}

void PSSetting()
{
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureResourceView);

	g_pImmediateContext->PSSetShaderResources(1, 1, &g_pNormalMapShaderResourceView);

	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
}

// **Change** 
void OMSetting()
{
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; 
	// ( SrcColor * blendFactor ) + (desColor * (1 - blendFactor))
	// + blendFactor를 조절하여 Fade In Out 기능을 구현할수 있다.
	// + 또는 색 강조도가능.
	// + 하지만 BlendFactor를 사용하면 Alpha 기능은 사용하지 못함.=
	//  BlendFactor 사용시 설정.
	// + blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
	// + blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR; // 1 - BlendFactor  // 렌더타켓
	
	g_pImmediateContext->OMSetBlendState(g_pAlphaBlendState, blendFactor, 0xffffffff);
}

void BeginPlay()
{
	InitDepthStencilBuffer();

	InitVertexBuffer();

	InitIndexBuffer();

	InitConstantBuffer();

	InitVertexShader();

	InitPixelShader();

	InitTexture();

	InitAlphaBlendState();

	IASetting();

	VSSetting();

	RSSetting();

	PSSetting();

	OMSetting();
}

void RenderBegin()
{
	float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Render()
{
	g_fRotaionAngle += 0.001f;

	Transform tf1;
	tf1.SetScale({ 1.0f, 1.0f, 1.0f });
	tf1.SetRotation({ 0.0f, 0.0f, g_fRotaionAngle });
	tf1.SetPosition({ 3.0f, -0.5f, 0.0f });
	UpdateConstantResource(tf1);
	g_pImmediateContext->DrawIndexed(36, 0, 0);

	Transform tf2;
	tf2.SetScale({ 1.0f, 1.0f, 1.0f });
	tf2.SetRotation({ 0.0f, 0.0f, g_fRotaionAngle });
	tf2.SetPosition({ 0.0f, -1.0f, 0.0f });
	UpdateConstantResource(tf2);
	g_pImmediateContext->DrawIndexed(36, 0, 0);
}

void RenderEnd()
{
	g_pSwapChain->Present(0, 0);
}

void Cleanup()
{
	if (g_pAlphaBlendState) g_pAlphaBlendState->Release();
	if (g_pNormalMapShaderResourceView) g_pNormalMapShaderResourceView->Release();
	if (g_pSamplerLinear) g_pSamplerLinear->Release();
	if (g_pTextureResourceView) g_pTextureResourceView->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pConstantBuffer) g_pConstantBuffer->Release();
	if (g_pIndexBuffer) g_pIndexBuffer->Release();
	if (g_pVertexBuffer) g_pVertexBuffer->Release();
	if (g_pInputLayout) g_pInputLayout->Release();
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