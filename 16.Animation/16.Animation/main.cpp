#include "stdafx.h"

#define ResolutionWidth 2560.0f
#define	ResolutionHeigh 1440.0f

constexpr int SELECT_MESH = 1;


// Init
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr; // Device Context
IDXGISwapChain* g_pSwapChain = nullptr; // 스왑 체인
ID3D11RenderTargetView* g_pRenderTargetView = nullptr; // 렌더 타켓 뷰
ID3D11DepthStencilView* g_pDepthStencilView = nullptr; // 깊이 스텐실 뷰

// Render
ID3D11InputLayout* g_pInputLayout = nullptr;
ID3D11Buffer* g_pConstantBuffer;
ID3D11Buffer* g_pAnimationBuffer;
ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;

// Texture
ID3D11ShaderResourceView* g_pTextureResourceView = nullptr;
ID3D11SamplerState* g_pSamplerLinear = nullptr;

// Normal Mapping
ID3D11ShaderResourceView* g_pNormalMapShaderResourceView = nullptr;

// Alpha Blend
ID3D11BlendState* g_pAlphaBlendState = nullptr;
ID3D11RasterizerState* g_pRasterizerState = nullptr;

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

struct AnimConstantBuffer
{
	DirectX::XMMATRIX animTransform[114];
};

FBXLoader* g_pFBXLoader = nullptr;
FBXMesh* g_pMesh = nullptr;
FBXAnimation* g_pAnimation = nullptr;

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

HRESULT InitMesh()
{
	D3D11_BUFFER_DESC bd;
	memset(&bd, 0x00, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = (UINT)(sizeof(SimpleVertex) * g_pMesh->pData_->meshVertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	memset(&InitData, 0x00, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = g_pMesh->pData_->meshVertices.data();

	HRESULT hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pMesh->pData_->pVertexBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	// 인덱스 버퍼
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = (UINT)(sizeof(WORD) * g_pMesh->pData_->meshIndices.size());
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	InitData.pSysMem = g_pMesh->pData_->meshIndices.data();
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pMesh->pData_->pIndexBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

HRESULT InitConstantBuffer()
{
	D3D11_BUFFER_DESC desc = {};
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

	D3D11_BUFFER_DESC animDesc = {};
	animDesc.Usage = D3D11_USAGE_DEFAULT;
	animDesc.ByteWidth = sizeof(AnimConstantBuffer);
	animDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	animDesc.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&animDesc, nullptr, &g_pAnimationBuffer);
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
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 68, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 84, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

HRESULT LoadWhiteTexture(ID3D11ShaderResourceView** ppOutSRV, UINT color)
{
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = 1.0f;
	texDesc.Height = 1.0f;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA ptexData = {};
	ptexData.pSysMem = (void*)&color;
	ptexData.SysMemPitch = sizeof(UINT);

	ID3D11Texture2D* ppTexture2D = nullptr;
	HRESULT hr = g_pd3dDevice->CreateTexture2D(&texDesc, &ptexData, &ppTexture2D);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return S_FALSE;
	}

	hr = g_pd3dDevice->CreateShaderResourceView(ppTexture2D, nullptr, ppOutSRV);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return S_FALSE;
	}
	ppTexture2D->Release();

	return S_OK;
}

HRESULT InitTexture()
{
	if (SELECT_MESH == 1)
	{
		const wchar_t* textureFile = L"../../Resource/fbx/Mesh/JUMPER_TextureUv1.png";
		HRESULT hr = LoadTextureWithDirectXTex(g_pd3dDevice, textureFile, false, &g_pTextureResourceView);
		if (FAILED(hr))
		{
			DEBUG_BREAK();
			return hr;
		}

		hr = LoadWhiteTexture(&g_pNormalMapShaderResourceView, 0xFFFFFFF);
		if (FAILED(hr))
		{
			DEBUG_BREAK();
			return hr;
		}
	}
	else
	{
		const wchar_t* textureFile = L"../../Resource/fbx/Mixamo/maria_diffuse.png";
		const wchar_t* normalFile = L"../../Resource/fbx/Mixamo/maria_normal.png";

		HRESULT hr = LoadTextureWithDirectXTex(g_pd3dDevice, textureFile, false, &g_pTextureResourceView);
		if (FAILED(hr))
		{
			DEBUG_BREAK();
			return hr;
		}

		hr = LoadTextureWithDirectXTex(g_pd3dDevice, normalFile, true, &g_pNormalMapShaderResourceView);
		if (FAILED(hr))
		{
			DEBUG_BREAK();
			return hr;
		}
	}

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;         // 선명하게 보여야할때 -> 16개
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // 반복
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // 반복
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; // 반복
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = g_pd3dDevice->CreateSamplerState(&samplerDesc, &g_pSamplerLinear);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}
	return hr;
}

HRESULT InitAlphaBlendState()
{
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; // 소스 알파값 지금 그려질 오브젝트
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 1 - 소스 알파값  // 렌더타켓
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HRESULT hr = g_pd3dDevice->CreateBlendState(&blendDesc, &g_pAlphaBlendState);
	return hr;
}

HRESULT InitRasterizerState()
{
	D3D11_RASTERIZER_DESC rasDesc = {};
	//rasDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_FRONT;
	//rasDesc.CullMode = D3D11_CULL_BACK;
	rasDesc.FrontCounterClockwise = false;

	HRESULT hr = g_pd3dDevice->CreateRasterizerState(&rasDesc, &g_pRasterizerState);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	return hr;
}


void UpdateConstantResource(const Transform& worldTransform)
{
	Float4 worldPosition = worldTransform.GetPosition();
	DirectX::XMMATRIX scale = worldTransform.GetScaleMatrix();
	DirectX::XMMATRIX rotation = worldTransform.GetRotationMatrix();
	DirectX::XMMATRIX position = worldTransform.GetPositionMatrix();
	DirectX::XMMATRIX world = scale * rotation * position;
	DirectX::XMVECTOR eye;
	DirectX::XMVECTOR target;
	DirectX::XMVECTOR up;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMFLOAT4 lightColor;
	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT3 spotPosition;
	DirectX::XMFLOAT3 spotDirection;

	if (SELECT_MESH == 1)
	{
		// [FBX Scene World Orientation] 
		// ------------------------------------
		// 	오른쪽(RIGHT) : -X
		// 	앞쪽(FRONT) : -Y
		// 	위쪽(UP) : +Z

		// 	------------------------------------
		// 	좌표계 : 오른손(Right - Handed)
		// 
		// 카메라 정면
		eye = DirectX::XMVectorSet(0, 20, 0, 1);
		target = DirectX::XMVectorAdd(eye, DirectX::XMVectorSet(0, -1, 0, 0)); // FRONT = -Y
		up = DirectX::XMVectorSet(0, 0, 1, 0);			// UP = +Z
		view = DirectX::XMMatrixLookAtRH(eye, target, up);
		projection = DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV4, ResolutionWidth / ResolutionHeigh, 0.01f, 1000.0f);

		// 카메라 측면
		//DirectX::XMVECTOR eye = DirectX::XMVectorSet(-20, 0, 0, 1);
		//DirectX::XMVECTOR target = DirectX::XMVectorSet(0, 0, 0, 0);
		//DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 0, 1, 0);			// UP = +Z
		//DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(eye, target, up);
		//DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV4, ResolutionWidth / ResolutionHeigh, 0.01f, 1000.0f);

		// 카메라 후면
		//DirectX::XMVECTOR eye = DirectX::XMVectorSet(0, -30, 0, 1);
		//DirectX::XMVECTOR target = DirectX::XMVectorSet(0, 0, 0, 0);
		//DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 0, 1, 0);			// UP = +Z
		//DirectX::XMMATRIX view = DirectX::XMMatrixLookAtRH(eye, target, up);
		//DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV4, ResolutionWidth / ResolutionHeigh, 0.01f, 1000.0f);

		// Spot Light
		lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		ambientColor = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.3f);
		spotPosition = DirectX::XMFLOAT3(-3.0f, 3.0f, 2.5f);
		spotDirection = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
	}
	else
	{
		// [FBX Scene World Orientation] 
		// ------------------------------------
		// 	앞쪽(FRONT) : +X
		// 	위쪽(UP) : +Y
		// 	오른쪽(RIGHT) : +Z
		// 	------------------------------------
		// 	좌표계 : 오른손(Right - Handed)

		// 카메라 정면
		eye = DirectX::XMVectorSet(-50, 0, 0, 1);
		target = DirectX::XMVectorAdd(eye, DirectX::XMVectorSet(1, 0, 0, 0)); // FRONT = -Y
		up = DirectX::XMVectorSet(0, 1, 0, 0);			// UP = +Z
		view = DirectX::XMMatrixLookAtRH(eye, target, up);
		projection = DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV4, ResolutionWidth / ResolutionHeigh, 0.01f, 1000.0f);


		lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		ambientColor = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 0.3f);
		spotPosition = DirectX::XMFLOAT3(-5.0f, 0.0f, 7.0f);
		spotDirection = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
	}

	float spotRange = 30.0f;
	float spotAngle = cosf(DirectX::XMConvertToRadians(30.0f));

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
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pMesh->pData_->pVertexBuffer, &stride, &offset);

	// 인덱스 버퍼 설정
	g_pImmediateContext->IASetIndexBuffer(g_pMesh->pData_->pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// 프리미티브 유형 설정
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void VSSetting()
{
	g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);

	// StartSlot : Buffer Slot
	// NumBuffers : Buffer가 2개 이상인 배열인 경우 설정.
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
	g_pImmediateContext->VSSetConstantBuffers(1, 1, &g_pAnimationBuffer);
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

	g_pImmediateContext->RSSetState(g_pRasterizerState);
}

void PSSetting()
{
	g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureResourceView);

	g_pImmediateContext->PSSetShaderResources(1, 1, &g_pNormalMapShaderResourceView);

	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
}

void OMSetting()
{
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	g_pImmediateContext->OMSetBlendState(g_pAlphaBlendState, blendFactor, 0xffffffff);
}

void BeginPlay()
{
	InitDepthStencilBuffer();

	g_pMesh = new FBXMesh;
	g_pAnimation = new FBXAnimation;
	g_pFBXLoader = new FBXLoader;


	if (SELECT_MESH == 1)
	{
		// 이걸로 했을때는 Light 적용이 됨.
		// vertices = 23507
		// indices = 52224
		// g_pFBXLoader->LoadMesh_V1(g_pMesh, "..\\..\\Resource\\Fbx\\Mesh\\JUMPER_MESH.FBX");
		// g_pFBXLoader->LoadAnimation(g_pAnimation, "..\\..\\Resource\\Fbx\\Animation\\JUMPER_IDLE.FBX");
		
		// 이걸로 했을때는 Light 적용이 안됨. 노말이 다른가??
		// vertices = 23349
		// indices = 52224
		g_pFBXLoader->LoadMesh_V2(g_pMesh, "..\\..\\Resource\\Fbx\\Mesh\\JUMPER_MESH.FBX");
		g_pFBXLoader->LoadAnimation(g_pAnimation, "..\\..\\Resource\\Fbx\\Animation\\JUMPER_IDLE.FBX");
	}
	else
	{
		// vertices = 8094
		// indices = 43698
		// g_pFBXLoader->LoadMesh_V1(g_pMesh, "..\\..\\Resource\\Fbx\\Mixamo\\Capoeira.fbx");
		// g_pFBXLoader->LoadAnimation(g_pAnimation, "..\\..\\Resource\\Fbx\\Mixamo\\Capoeira.fbx");


		// vertices = 9584
		// indices = 43698
		g_pFBXLoader->LoadMesh_V2(g_pMesh, "..\\..\\Resource\\Fbx\\Mixamo\\Capoeira.fbx");
		g_pFBXLoader->LoadAnimation(g_pAnimation, "..\\..\\Resource\\Fbx\\Mixamo\\Capoeira.fbx");
	}

	InitMesh();

	InitConstantBuffer();

	InitVertexShader();

	InitPixelShader();

	InitTexture();

	InitAlphaBlendState();

	InitRasterizerState();

	IASetting();

	VSSetting();

	RSSetting();

	PSSetting();

	OMSetting();
}

double currentTime = 0.0;
bool loop = true;

void UpdateAnimation
(
	FBXMesh& player,
	const FBXAnimation& animation,
	double deltaTime,
	std::vector<FbxAMatrix>& outFinalBoneMatrices
)
{
	const AnimationClip& clip = animation.animationClip_;

	// 1. 시간 진행
	currentTime += deltaTime;
	if (loop)
	{
		while (currentTime > clip.duration)
		{
			currentTime -= clip.duration;
		}
	}
	else
	{
		if (currentTime > clip.duration)
		{
			currentTime = clip.duration;
		}
	}

	const int boneCount = player.bones_.size();
	outFinalBoneMatrices.resize(boneCount);

	// 2. 각 본에 대해 애니메이션 행렬 계산
	for (int i = 0; i < boneCount; ++i)
	{
		const BoneAnimation& boneAnim = clip.boneAnimations[i];

		// Keyframe 2개 찾기 (Nearest Sampling)
		const BoneKeyframe* k0 = nullptr;
		const BoneKeyframe* k1 = nullptr;

		for (size_t k = 0; k + 1 < boneAnim.keyframes.size(); ++k)
		{
			if (currentTime >= boneAnim.keyframes[k].time.GetSecondDouble()
				&& currentTime <= boneAnim.keyframes[k + 1].time.GetSecondDouble())
			{
				k0 = &boneAnim.keyframes[k];
				k1 = &boneAnim.keyframes[k + 1];
				break;
			}
		}

		// fallback
		if (!k0 || !k1)
		{
			outFinalBoneMatrices[i] = player.bones_[i].boneBindPose.Inverse();
			continue;
		}

		double t0 = k0->time.GetSecondDouble();
		double t1 = k1->time.GetSecondDouble();
		double alpha = (currentTime - t0) / (t1 - t0);

		// 3. Global Transform 보간
		FbxVector4 tA = k0->globalTransform.GetT();
		FbxVector4 tB = k1->globalTransform.GetT();

		FbxQuaternion rA = k0->globalTransform.GetQ();
		FbxQuaternion rB = k1->globalTransform.GetQ();

		FbxVector4 sA = k0->globalTransform.GetS();
		FbxVector4 sB = k1->globalTransform.GetS();

		FbxVector4 T = tA + (tB - tA) * alpha;
		FbxQuaternion Q;
		Q = rA.Slerp(rB, alpha);
		FbxVector4 S = sA + (sB - sA) * alpha;

		FbxAMatrix animatedGlobal;
		animatedGlobal.SetTQS(T, Q, S);


		// 4. Skinning 최종 행렬
		outFinalBoneMatrices[i] = animatedGlobal * player.bones_[i].boneBindPose.Inverse();
	}
}

DirectX::XMFLOAT4X4 ConvertToXMFLOAT4X4(const FbxAMatrix& m)
{
	DirectX::XMFLOAT4X4 result;

	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < 4; ++col)
		{
			result.m[row][col] = static_cast<float>(m[row][col]);
		}
	}
	return result;
}

void Update()
{
	std::vector<FbxAMatrix> finalBoneMatrices;
	UpdateAnimation(*g_pMesh, *g_pAnimation, 0.01, finalBoneMatrices);

	AnimConstantBuffer cb = {};
	for (int i = 0; i < finalBoneMatrices.size(); ++i)
	{
		DirectX::XMFLOAT4X4 M = ConvertToXMFLOAT4X4(finalBoneMatrices[i]);
		DirectX::XMMATRIX tmpM = XMLoadFloat4x4(&M);
		cb.animTransform[i] = XMMatrixTranspose(tmpM);
	}

	g_pImmediateContext->UpdateSubresource(g_pAnimationBuffer, 0, nullptr, &cb, 0, 0);
}

void RenderBegin()
{
	float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}


void Render()
{
	if (SELECT_MESH == 1)
	{
		// 	오른쪽(RIGHT) : -X
		// 	앞쪽(FRONT) : -Y
		// 	위쪽(UP) : +Z
		Transform tf1;
		tf1.SetScale({ 1.0f, 1.0f, 1.0f });
		tf1.SetRotation({ 0.0f, 0.0f, 3.0f });
		tf1.SetPosition({ 3.0f,  0.0f, 0.0f });
		UpdateConstantResource(tf1);
		g_pImmediateContext->DrawIndexed(g_pMesh->pData_->meshIndices.size(), 0, 0);


		Transform tf2;
		tf2.SetScale({ 1.0f, 1.0f, 1.0f });
		tf2.SetRotation({ 0.0f, 0.0f, 3.0f });
		tf2.SetPosition({ -3.0f, 0.0f, 0.0f });
		UpdateConstantResource(tf2);
		g_pImmediateContext->DrawIndexed(g_pMesh->pData_->meshIndices.size(), 0, 0);
	}
	else
	{
		// 	앞쪽(FRONT) : +X
		// 	위쪽(UP) : +Y
		// 	오른쪽(RIGHT) : +Z
		Transform tf1;
		tf1.SetScale({ 0.1f, 0.1f, 0.1f });
		tf1.SetRotation({ 0.0f, -1.9f, 0.0f });
		tf1.SetPosition({ 0.0f, -8.0f, -10.0f });
		UpdateConstantResource(tf1);
		g_pImmediateContext->DrawIndexed(g_pMesh->pData_->meshIndices.size(), 0, 0);

		Transform tf2;
		tf2.SetScale({ 0.1f, 0.1f, 0.1f });
		tf2.SetRotation({ 0.0f, -1.9f, 0.0f });
		tf2.SetPosition({ 0.0f, -8.0f, 10.0f });
		UpdateConstantResource(tf2);
		g_pImmediateContext->DrawIndexed(g_pMesh->pData_->meshIndices.size(), 0, 0);
	}
}

void RenderEnd()
{
	g_pSwapChain->Present(0, 0);
}

void Cleanup()
{
	if (g_pFBXLoader)
	{
		delete g_pFBXLoader;
		g_pFBXLoader = nullptr;
	}

	if (g_pMesh)
	{
		delete g_pMesh;
		g_pMesh = nullptr;
	}

	if (g_pRasterizerState) g_pRasterizerState->Release();
	if (g_pAlphaBlendState) g_pAlphaBlendState->Release();
	if (g_pNormalMapShaderResourceView) g_pNormalMapShaderResourceView->Release();
	if (g_pSamplerLinear) g_pSamplerLinear->Release();
	if (g_pTextureResourceView) g_pTextureResourceView->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pConstantBuffer) g_pConstantBuffer->Release();
	if (g_pAnimationBuffer) g_pAnimationBuffer->Release();
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
			Update();

			RenderBegin();

			Render();

			RenderEnd();
		}
	}

	Cleanup();

	return (int)msg.wParam;
}