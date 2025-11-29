#include "stdafx.h"


static float ResolutionWidth = 2560.0f;
static float ResolutionHeight = 1440.0f;

// Imgui Variable
bool g_ShowExitPopup = false;
bool g_bShow_demo_window = true;
bool g_bShow_another_window = false;
ImVec4 g_Imgui_Clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


struct Object
{
	Transform transform;
};

struct Camera
{
	DirectX::XMVECTOR camPos;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;
};

Object* g_pObject = nullptr;
Camera* g_pCamera = nullptr;


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

ID3D11VertexShader* g_pVertexShader = nullptr;
ID3D11PixelShader* g_pPixelShader = nullptr;

ID3D11Buffer* g_pCBTransform = nullptr;
ID3D11Buffer* g_pCBLight = nullptr;
ID3D11Buffer* g_pCBMaterial = nullptr;

// Texture
//ID3D11Texture2D* g_pTextureResource = nullptr;
ID3D11ShaderResourceView* g_pTextureResourceView = nullptr;
ID3D11SamplerState* g_pSamplerLinear = nullptr;

// Normal Mapping
ID3D11ShaderResourceView* g_pNormalMapShaderResourceView = nullptr;

// Alpha Blend
ID3D11BlendState* g_pAlphaBlendState = nullptr;

//Rasterizer
ID3D11RasterizerState* g_pRasterizerState = nullptr;

struct SimpleVertex {

	Float3 position;
	Float4 color;
	Float3 normal;
	Float2 UV;
	Float4 tangent;

	SimpleVertex(Float3 pos, Float4 tmpcolor, Float3 tmpnormal, Float2 uv, Float4 tan)
		: position(pos), color(tmpcolor), normal(tmpnormal), UV(uv), tangent(tan) {
	}

	SimpleVertex(Float3 pos, Float4 tmpcolor, Float3 tmpnormal, Float2 uv)
		: position(pos), color(tmpcolor), normal(tmpnormal), UV(uv), tangent() {
	}
};

struct CBTransform
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX worldInvTranspose;

	Float4 camPos;
};

struct CBLight
{
	Float4 lightDiffuse;
	Float4 lightSpecular;
	Float4 lightAmbient;

	Float3 lightDir;
	float pad0;
};

struct CBMaterial
{
	Float4 materialAmbient;
	Float4 materialDiffuse;
	Float4 materialSpecular;
	Float4 materialEmissive;

	float shininess;
	Float3 materialPad;
};

SimpleVertex vertices[] = {
	// 아래 (-Z)
	{Float3(-0.5f, -0.5f, -0.5f), Float4(1,0,0,1), Float3(0,0,-1), Float2(0,1), Float4(-1,0,0,1)},
	{Float3(-0.5f,  0.5f, -0.5f), Float4(1,0,0,1), Float3(0,0,-1), Float2(0,0), Float4(-1,0,0,1)},
	{Float3(0.5f,  0.5f, -0.5f),  Float4(1,0,0,1), Float3(0,0,-1), Float2(1,0), Float4(-1,0,0,1)},
	{Float3(0.5f, -0.5f, -0.5f),  Float4(1,0,0,1), Float3(0,0,-1), Float2(1,1), Float4(-1,0,0,1)},
	// 위 (+Z)
	{Float3(-0.5f, -0.5f,  0.5f), Float4(1,0,0,1), Float3(0,0,1), Float2(1,1),Float4(1,0,0,1)},
	{Float3(0.5f, -0.5f,  0.5f),  Float4(1,0,0,1), Float3(0,0,1), Float2(0,1),Float4(1,0,0,1)},
	{Float3(0.5f,  0.5f,  0.5f),  Float4(1,0,0,1), Float3(0,0,1), Float2(0,0),Float4(1,0,0,1)},
	{Float3(-0.5f,  0.5f,  0.5f), Float4(1,0,0,1), Float3(0,0,1), Float2(1,0),Float4(1,0,0,1)},
	// 앞 (-X)
	{Float3(-0.5f, -0.5f,  0.5f), Float4(0,1,0,1), Float3(-1,0,0), Float2(0,1),Float4(0,0,1,1)},
	{Float3(-0.5f,  0.5f,  0.5f), Float4(0,1,0,1), Float3(-1,0,0), Float2(0,0),Float4(0,0,1,1)},
	{Float3(-0.5f,  0.5f, -0.5f), Float4(0,1,0,1), Float3(-1,0,0), Float2(1,0),Float4(0,0,1,1)},
	{Float3(-0.5f, -0.5f, -0.5f), Float4(0,1,0,1), Float3(-1,0,0), Float2(1,1),Float4(0,0,1,1)},
	// 뒤 (+X)
	{Float3(0.5f, -0.5f, -0.5f), Float4(0,1,0,1), Float3(1,0,0), Float2(0,1),Float4(0,0,-1,1)},
	{Float3(0.5f,  0.5f, -0.5f), Float4(0,1,0,1), Float3(1,0,0), Float2(0,0),Float4(0,0,-1,1)},
	{Float3(0.5f,  0.5f,  0.5f), Float4(0,1,0,1), Float3(1,0,0), Float2(1,0),Float4(0,0,-1,1)},
	{Float3(0.5f, -0.5f,  0.5f), Float4(0,1,0,1), Float3(1,0,0), Float2(1,1),Float4(0,0,-1,1)},
	// 오른쪽 (+Y)
	{Float3(-0.5f,  0.5f, -0.5f), Float4(0,0,1,1), Float3(0,1,0), Float2(0,1),Float4(1,0,0,1)},
	{Float3(-0.5f,  0.5f,  0.5f), Float4(0,0,1,1), Float3(0,1,0), Float2(0,0),Float4(1,0,0,1)},
	{Float3(0.5f,  0.5f,  0.5f),  Float4(0,0,1,1), Float3(0,1,0), Float2(1,0),Float4(1,0,0,1)},
	{Float3(0.5f,  0.5f, -0.5f),  Float4(0,0,1,1), Float3(0,1,0), Float2(1,1),Float4(1,0,0,1)},
	// 왼쪽 (-Y)
	{Float3(-0.5f, -0.5f,  0.5f), Float4(0,0,1,1), Float3(0,-1,0), Float2(1,1),Float4(-1,0,1,1)},
	{Float3(-0.5f, -0.5f, -0.5f), Float4(0,0,1,1), Float3(0,-1,0), Float2(0,1),Float4(-1,0,1,1)},
	{Float3(0.5f, -0.5f, -0.5f),  Float4(0,0,1,1), Float3(0,-1,0), Float2(0,0),Float4(-1,0,1,1)},
	{Float3(0.5f, -0.5f,  0.5f),  Float4(0,0,1,1), Float3(0,-1,0), Float2(1,0),Float4(-1,0,1,1)},
};

WORD indices[] = {
	0,1,2, 0,2,3,       // Front
	4,5,6, 4,6,7,       // Back
	8,9,10, 8,10,11,    // Left
	12,13,14, 12,14,15, // Right
	16,17,18, 16,18,19, // Top
	20,21,22, 20,22,23  // Bottom
};

std::vector<SimpleVertex> SpereVertices;

std::vector<WORD> SpereIndices;

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

bool CreateSphere(std::vector<SimpleVertex>* outVertices, std::vector<WORD>* outIndices, float radius = 0.5f)
{
	if (nullptr == outVertices || nullptr == outIndices)
	{
		return false;
	}

	const int SPHERE_STACKS = 20;
	const int SPHERE_SLICES = 20;

	for (int stack = 0; stack <= SPHERE_STACKS; ++stack)
	{
		float phi = MATH::PI * stack / SPHERE_STACKS; // 0 ~ PI
		float z = cosf(phi);
		float r = sinf(phi);

		for (int slice = 0; slice <= SPHERE_SLICES; ++slice)
		{
			float theta = 2.0f * MATH::PI * slice / SPHERE_SLICES; // 0 ~ 2PI
			float x = r * cosf(theta);
			float y = r * sinf(theta);

			Float3 pos = Float3(x * radius, y * radius, z * radius);
			Float3 normal = Float3(x, y, z);
			Float4 color = Float4(
				0.5f + 0.5f * x,
				0.5f + 0.5f * y,
				0.5f + 0.5f * z,
				1.0f
			);

			float u = (float)slice / SPHERE_SLICES;
			float v = (float)stack / SPHERE_STACKS;

			Float2 uv = Float2(u, v);
			outVertices->push_back({ pos, color, normal, uv });
		}
	}

	for (int stack = 0; stack < SPHERE_STACKS; ++stack)
	{
		for (int slice = 0; slice < SPHERE_SLICES; ++slice)
		{
			int first = (stack * (SPHERE_SLICES + 1)) + slice;
			int second = first + SPHERE_SLICES + 1;

			// 삼각형 1
			outIndices->push_back((WORD)first);
			outIndices->push_back((WORD)second);
			outIndices->push_back((WORD)(first + 1));

			// 삼각형 2
			outIndices->push_back((WORD)second);
			outIndices->push_back((WORD)(second + 1));
			outIndices->push_back((WORD)(first + 1));
		}
	}

	std::vector<Float3> tan1(outVertices->size(), Float3(0, 0, 0));
	std::vector<Float3> tan2(outVertices->size(), Float3(0, 0, 0));

	for (size_t n = 0; n < outIndices->size(); n += 3)
	{
		WORD i0 = (*outIndices)[n];
		WORD i1 = (*outIndices)[n + 1];
		WORD i2 = (*outIndices)[n + 2];

		const Float3& p0 = (*outVertices)[i0].position;
		const Float3& p1 = (*outVertices)[i1].position;
		const Float3& p2 = (*outVertices)[i2].position;

		const Float2& uv0 = (*outVertices)[i0].UV;
		const Float2& uv1 = (*outVertices)[i1].UV;
		const Float2& uv2 = (*outVertices)[i2].UV;

		//DirectX::XMVECTOR v0 = p0.dxVector;
		//DirectX::XMVECTOR v1 = p1.dxVector;
		//DirectX::XMVECTOR v2 = p2.dxVector;

		//DirectX::XMVECTOR uvv0 = uv0.dxVector;
		//DirectX::XMVECTOR uvv1 = uv1.dxVector;
		//DirectX::XMVECTOR uvv2 = uv2.dxVector;

		float x1 = p1.X - p0.X;
		float y1 = p1.Y - p0.Y;
		float z1 = p1.Z - p0.Z;

		float x2 = p2.X - p0.X;
		float y2 = p2.Y - p0.Y;
		float z2 = p2.Z - p0.Z;

		float s1 = uv1.X - uv0.X;
		float t1 = uv1.Y - uv0.Y;

		float s2 = uv2.X - uv0.X;
		float t2 = uv2.Y - uv0.Y;

		float r = (s1 * t2 - s2 * t1);
		if (fabs(r) < 1e-6f)
		{
			r = 1.0f;
		}

		float invR = 1.0f / r;

		Float3 sdir(
			(t2 * x1 - t1 * x2) * invR,
			(t2 * y1 - t1 * y2) * invR,
			(t2 * z1 - t1 * z2) * invR
		);

		Float3 tdir(
			(s1 * x2 - s2 * x1) * invR,
			(s1 * y2 - s2 * y1) * invR,
			(s1 * z2 - s2 * z1) * invR
		);

		tan1[i0].X += sdir.X;
		tan1[i0].Y += sdir.Y;
		tan1[i0].Z += sdir.Z;

		tan1[i1].X += sdir.X;
		tan1[i1].Y += sdir.Y;
		tan1[i1].Z += sdir.Z;

		tan1[i2].X += sdir.X;
		tan1[i2].Y += sdir.Y;
		tan1[i2].Z += sdir.Z;

		tan2[i0].X += tdir.X;
		tan2[i0].Y += tdir.Y;
		tan2[i0].Z += tdir.Z;

		tan2[i1].X += tdir.X;
		tan2[i1].Y += tdir.Y;
		tan2[i1].Z += tdir.Z;

		tan2[i2].X += tdir.X;
		tan2[i2].Y += tdir.Y;
		tan2[i2].Z += tdir.Z;
	}

	for (size_t n = 0; n < outVertices->size(); ++n)
	{
		const Float3& normal = (*outVertices)[n].normal;
		const Float3& tangent = tan1[n];
		const Float3& tangent2 = tan2[n];

		float dotResult = 0.0f;
		VectorDot(dotResult, normal, tangent);

		Float3 scaleResult;
		VectorScale(scaleResult, normal, dotResult);

		Float3 subResult;
		VectorSub(subResult, tangent, scaleResult);

		Float3  finalTangent;
		VectorNormalize(finalTangent, subResult);

		Float3 crossResult;
		VectorCross(crossResult, normal, tangent);

		float dotResult2 = 0.0f;
		VectorDot(dotResult2, crossResult, tangent2);

		float handedness = (dotResult2 < 0.0f) ? -1.0f : 1.0f;

		(*outVertices)[n].tangent = Float4(finalTangent.X, finalTangent.Y, finalTangent.Z, handedness);

	}
	return true;
}

HRESULT InitDeviceAndSwapChain(HWND hWnd, IDXGIAdapter* pBestAdapter)
{
	// 스왑 체인 구조체를 초기화 해야 함
	DXGI_SWAP_CHAIN_DESC sd;
	memset(&sd, 0x00, sizeof(sd));
	sd.BufferCount = 1; // 백 버퍼의 수
	sd.BufferDesc.Width = ResolutionWidth; // 백 버퍼의 너비
	sd.BufferDesc.Height = ResolutionHeight; // 백 버퍼의 높이
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
	Desc.Height = ResolutionHeight;
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

HRESULT InitImgui(HWND hWnd, float dpiScale)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Control
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 24.0f, nullptr, io.Fonts->GetGlyphRangesKorean());
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(dpiScale);
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pImmediateContext);

	ImGuizmo::SetOrthographic(false);

	return S_OK;
}

HRESULT InitCamera()
{
	g_pCamera = new Camera;
	g_pCamera->camPos = DirectX::XMVectorSet(0.0f, 5.0f, 0.0f, 0.0f);
	g_pCamera->view= DirectX::XMMatrixLookToLH(g_pCamera->camPos, DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	g_pCamera->proj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, ResolutionWidth / ResolutionHeight, 0.01f, 100.0f);
	
	return S_OK;
}

HRESULT InitObject()
{
	g_pObject = new Object;
	g_pObject->transform.SetScale({1.0f, 1.0f, 1.0f});
	g_pObject->transform.SetPosition({0.0f, 0.0f, 0.0f});
	g_pObject->transform.SetRotation({ 0.0f, 0.0f, 0.0f});

	return S_OK;
}

HRESULT InitVertexBuffer()
{
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0x00, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(SimpleVertex) * SpereVertices.size();
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	memset(&InitData, 0x00, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = SpereVertices.data();

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
	//desc.ByteWidth = sizeof(indices);
	desc.ByteWidth = sizeof(WORD) * SpereIndices.size();
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	memset(&InitData, 0x00, sizeof(D3D11_SUBRESOURCE_DATA));
	//InitData.pSysMem = indices;
	InitData.pSysMem = SpereIndices.data();

	HRESULT hr = g_pd3dDevice->CreateBuffer(&desc, &InitData, &g_pIndexBuffer);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}
	return S_OK;
}

HRESULT InitConstantBuffer(UINT byteWidth, ID3D11Buffer** constantBuffer)
{
	if (nullptr != *constantBuffer)
	{
		(*constantBuffer)->Release();
		(*constantBuffer) = nullptr;
	}

	D3D11_BUFFER_DESC desc;
	memset(&desc, 0x00, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;

	HRESULT hr = g_pd3dDevice->CreateBuffer(&desc, nullptr, constantBuffer);
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
	const wchar_t* textureFile = L"../../Resource/Bricks_2K/Bricks_Color.png";
	//const wchar_t* textureFile = L"../../Resource/Bricks_4K/Bricks_Color.png";
	//const wchar_t* textureFile = L"../../Resource/Stones_2K/Stones_Color.png";
	//const wchar_t* textureFile = L"../../Resource/Stones_4K/Stones_Color.png";
	//const wchar_t* textureFile = L"../../Resource/Ragnarok_Online_Acolyte.png";
	//const wchar_t* textureFile = L"../../Resource/BrickTexture.jpg";
	HRESULT hr = LoadTextureWithDirectXTex(g_pd3dDevice, textureFile, false, &g_pTextureResourceView);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}

	const wchar_t* normalFile = L"../../Resource/Bricks_2K/Bricks_NormalDX.png";
	//const wchar_t* normalFile = L"../../Resource/Bricks_4K/Bricks_NormalDX.png";
	//const wchar_t* normalFile = L"../../Resource/Stones_2K/Stones_NormalDX.png";
	//const wchar_t* normalFile = L"../../Resource/Stones_4K/Stones_NormalDX.png";
	//const wchar_t* normalFile = L"../../Resource/BrickNormal.jpg";
	hr = LoadTextureWithDirectXTex(g_pd3dDevice, normalFile, true, &g_pNormalMapShaderResourceView);
	if (FAILED(hr))
	{
		DEBUG_BREAK();
		return hr;
	}



	D3D11_SAMPLER_DESC samplerDesc = {};
	// Point 샘플링
	// samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;	// 0 ~ 1 최근접  -> 계단형태로 보임 3D
	// Linear 샘플링
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;	// 0 ~ 1 선형 U V  -> 주위 4개의 픽셀 
	// Anisotropic 샘플링
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;         // 선명하게 보여야할때 -> 16개
	samplerDesc.MaxAnisotropy = 16;

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

	// BlendFactor : 색을 강조하거나, 화면 전체 페이드 같은 특수 효과를 만들 때 사용. ->OMSetBlendState()에서 BlendFactor 값 인자로 넘김.
	//blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
	//blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR; // 1 - BlendFactor  // 렌더타켓

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

	//rasDesc.CullMode = D3D11_CULL_FRONT;
	rasDesc.CullMode = D3D11_CULL_BACK;

	rasDesc.FrontCounterClockwise = false;

	//rasDesc.DepthBias;
	//rasDesc.DepthBiasClamp;
	//rasDesc.SlopeScaledDepthBias;
	//rasDesc.DepthClipEnable;
	//rasDesc.ScissorEnable;
	//rasDesc.MultisampleEnable;
	//rasDesc.AntialiasedLineEnable;

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
	DirectX::XMMATRIX worldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, world));

	// 월드, 뷰, 프로젝션 행렬 설정
	// world는 오브젝트마다 고유의 값이며, 각각의 오브젝트의 Transform 을 적용해야함.
	


	CBTransform cbTransform;
	cbTransform.world = DirectX::XMMatrixTranspose(world);
	cbTransform.view = DirectX::XMMatrixTranspose(g_pCamera->view);
	cbTransform.projection = DirectX::XMMatrixTranspose(g_pCamera->proj);
	cbTransform.worldInvTranspose = XMMatrixTranspose(worldInvTranspose);
	cbTransform.camPos =
	{
		DirectX::XMVectorGetX(g_pCamera->camPos),
		DirectX::XMVectorGetY(g_pCamera->camPos),
		DirectX::XMVectorGetZ(g_pCamera->camPos),
		DirectX::XMVectorGetW(g_pCamera->camPos)
	};

	// 일반 빛 색상.
	CBLight cbLight;
	cbLight.lightAmbient = { 0.15f, 0.15f, 0.15f, 1.0f }; // 주변광 색상 (조명 없는 부분)
	cbLight.lightDiffuse = { 1.0f, 1.0f, 1.0f, 1.0f }; // 확산광 색상
	cbLight.lightSpecular = { 1.0f, 1.0f, 1.0f, 1.0f }; // 하이라이트 색상
	cbLight.lightDir = { 0.0f, 0.0f, -1.0f };       // 광원 방향 (정규화)

	// 따뜻한 햇빛
	//CBLight cbLight;
	//cbLight.lightAmbient = { 0.2f, 0.18f, 0.15f, 1.0f }; // 약간 노란 주변광
	//cbLight.lightDiffuse = { 1.0f, 0.95f, 0.9f, 1.0f };   // 따뜻한 햇빛
	//cbLight.lightSpecular = { 1.0f, 0.95f, 0.9f, 1.0f };   // 하이라이트 동일
	//cbLight.lightDir = { 0.0f, 0.0f, -1.0f };

	// 비금속 (돌, 플라스틱)
	CBMaterial cbMaterial;
	cbMaterial.materialDiffuse = { 0.8f, 0.2f, 0.2f, 1.0f }; // 밝은 빨강
	cbMaterial.materialSpecular = { 1.0f, 1.0f, 1.0f, 1.0f }; // 흰색 하이라이트
	cbMaterial.materialAmbient = { 0.2f, 0.05f, 0.05f, 1.0f }; // 어두운 빨강
	cbMaterial.materialEmissive = { 0.0f, 0.0f, 0.0f, 1.0f }; // 발광 없음
	cbMaterial.shininess = 16.0f;

	//금속(Gold, Copper, Iron 등)
	//CBMaterial cbMaterial;
	//cbMaterial.materialDiffuse = { 1.0f, 0.8f, 0.0f, 1.0f }; // 거의 없음
	//cbMaterial.materialSpecular = { 1.0f, 0.8f, 0.0f, 1.0f }; // 금빛 반사
	//cbMaterial.materialAmbient = { 0.0f, 0.0f, 0.0f, 1.0f }; // 그림자 속 거의 없음
	//cbMaterial.materialEmissive = { 0.0f, 0.0f, 0.0f, 1.0f }; // 발광 없음
	//cbMaterial.shininess = 64.0f;

	//강철 (Steel)
	//CBMaterial cbMaterial;
	//cbMaterial.materialDiffuse = { 0.3f, 0.3f, 0.3f, 1.0f }; // 어두운 회색
	//cbMaterial.materialSpecular = { 0.8f, 0.8f, 0.8f, 1.0f }; // 밝은 하이라이트
	//cbMaterial.materialAmbient = { 0.3f, 0.3f, 0.3f, 1.0f }; // 그림자
	//cbMaterial.materialEmissive = { 0.0f, 0.0f, 0.0f, 1.0f }; // 발광 없음
	//cbMaterial.shininess = 128.0f;



	g_pImmediateContext->UpdateSubresource(g_pCBTransform, 0, nullptr, &cbTransform, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pCBLight, 0, nullptr, &cbLight, 0, 0);
	g_pImmediateContext->UpdateSubresource(g_pCBMaterial, 0, nullptr, &cbMaterial, 0, 0);
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
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCBTransform);
}

void RSSetting()
{
	// 뷰 포트 설정
	D3D11_VIEWPORT vp;
	vp.Width = ResolutionWidth;
	vp.Height = ResolutionHeight;
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

	//g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBTransform);

	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pCBTransform);
	g_pImmediateContext->PSSetConstantBuffers(1, 1, &g_pCBLight);
	g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pCBMaterial);

	g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureResourceView);

	g_pImmediateContext->PSSetShaderResources(1, 1, &g_pNormalMapShaderResourceView);

	g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
}

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
	CreateSphere(&SpereVertices, &SpereIndices, 0.5f);

	InitCamera();

	InitObject();

	InitDepthStencilBuffer();

	InitVertexBuffer();

	InitIndexBuffer();

	InitConstantBuffer(sizeof(CBTransform), &g_pCBTransform);
	InitConstantBuffer(sizeof(CBLight), &g_pCBLight);
	InitConstantBuffer(sizeof(CBMaterial), &g_pCBMaterial);

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

void RenderBegin()
{
	float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Render()
{
	g_fRotaionAngle += 0.002f;
	
	Transform tf1 = g_pObject->transform;
	UpdateConstantResource(tf1);

	g_pImmediateContext->DrawIndexed(SpereIndices.size(), 0, 0);

	//Transform tf2;
	//tf2.SetScale({ 1.0f, 1.0f, 1.0f });
	//tf2.SetRotation({ 0.0f, 0.0f, g_fRotaionAngle });
	//tf2.SetPosition({ 0.0f, 0.0f, 0.0f });
	//UpdateConstantResource(tf2);
	//g_pImmediateContext->DrawIndexed(SpereIndices.size(), 0, 0);
}

void ImguiExit(HWND hWnd)
{
	// ESC 키 감지
	if (ImGui::IsKeyPressed(ImGuiKey_Escape))
	{
		g_ShowExitPopup = true;
		ImGui::OpenPopup("Exit Program");
	}

	// 종료 팝업
	if (g_ShowExitPopup)
	{
		ImGui::SetNextWindowSize(ImVec2(300, 150));
		if (ImGui::BeginPopupModal("Exit Program", NULL, ImGuiWindowFlags_NoResize))
		{
			ImGui::Text(" Do you want to exit the program ? ");
			ImGui::Spacing();
			ImGui::Spacing();

			if (ImGui::Button("Quit", ImVec2(120, 0)))
			{
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				g_ShowExitPopup = false;
			}

			ImGui::EndPopup();
		}
	}
}

void ImguiDemo()
{
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (g_bShow_demo_window)
	{
		ImGui::ShowDemoWindow(&g_bShow_demo_window);
	}

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	{
		static float f = 0.0f;
		static int counter = 0;
		ImGuiIO& io = ImGui::GetIO();

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &g_bShow_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &g_bShow_another_window);
		ImGui::SliderFloat("float", &f, 0.0, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&g_Imgui_Clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))
		{ // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		}
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (g_bShow_another_window)
	{
		ImGui::Begin("Another Window", &g_bShow_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
		{
			g_bShow_another_window = false;
		}
		ImGui::End();
	}
}

void ImguiDockingSpaceDemo()
{
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
	{
		window_flags |= ImGuiWindowFlags_NoBackground;
	}

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	if (!opt_padding)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	}

	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
	if (!opt_padding)
	{
		ImGui::PopStyleVar();
	}

	if (opt_fullscreen)
	{
		ImGui::PopStyleVar(2);
	}

	// Submit the DockSpace
	// REMINDER: THIS IS A DEMO FOR ADVANCED USAGE OF DockSpace()!
	// MOST REGULAR APPLICATIONS WILL SIMPLY WANT TO CALL DockSpaceOverViewport(). READ COMMENTS ABOVE.
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
	else
	{
		// ShowDockingDisabledMessage();
	}

	// Show demo options and help
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Options"))
		{
			// Disabling fullscreen would allow the window to be moved to the front of other windows,
			// which we can't undo at the moment without finer window depth/z control.
			ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
			ImGui::MenuItem("Padding", NULL, &opt_padding);
			ImGui::Separator();

			if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode; }
			if (ImGui::MenuItem("Flag: NoDockingSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit; }
			if (ImGui::MenuItem("Flag: NoUndocking", "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking; }
			if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
			if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
			if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
			ImGui::Separator();

			if (ImGui::MenuItem("Close", NULL, false, nullptr != NULL))
			{
				//*p_open = false;
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help"))
		{
			ImGui::TextUnformatted(
				"This demo has nothing to do with enabling docking!" "\n"
				"This demo only demonstrate the use of ImGui::DockSpace() which allows you to manually\ncreate a docking node _within_ another window." "\n"
				"Most application can simply call ImGui::DockSpaceOverViewport() and be done with it.");
			ImGui::Separator();
			ImGui::TextUnformatted("When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
				"- Drag from window title bar or their tab to dock/undock." "\n"
				"- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
				"- Hold SHIFT to disable docking (if io.ConfigDockingWithShift == false, default)" "\n"
				"- Hold SHIFT to enable docking (if io.ConfigDockingWithShift == true)");
			ImGui::Separator();
			ImGui::TextUnformatted("More details:"); ImGui::Bullet(); ImGui::SameLine(); ImGui::TextLinkOpenURL("Docking Wiki page", "https://github.com/ocornut/imgui/wiki/Docking");
			ImGui::BulletText("Read comments in ShowExampleAppDockSpace()");
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	ImGui::End();
}

void MatrixToTransform(const DirectX::XMMATRIX& mat, Transform& out)
{
	DirectX::XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, mat);
	
	//// Scale 추출
	//float sx = sqrtf(m._11 * m._11 + m._12 * m._12 + m._13 * m._13);
	//float sy = sqrtf(m._21 * m._21 + m._22 * m._22 + m._23 * m._23);
	//float sz = sqrtf(m._31 * m._31 + m._32 * m._32 + m._33 * m._33);
	//out.SetScale({ sx, sy, sz});

	//// Rotation 추출
	//float ry = atan2f(-m._13, sqrtf(m._11 * m._11 + m._12 * m._12));
	//float rx = atan2f(m._23, m._33);
	//float rz = atan2f(m._12, m._11);
	//rx = DirectX::XMConvertToDegrees(rx);
	//ry = DirectX::XMConvertToDegrees(ry);
	//rz = DirectX::XMConvertToDegrees(rz);
	//out.SetRotation({ rx, ry, rz });

	// Translation 추출
	//out.Position = { m._41, m._42, m._43 };
	out.SetPosition({ m._41, m._42, m._43 });
}

void ImguiPickingTransform()
{
	ImGui::Begin("Transform Controller");
	ImGui::Text("Position: %.2f %.2f %.2f", 
		g_pObject->transform.GetPosition().X, 
		g_pObject->transform.GetPosition().Y, 
		g_pObject->transform.GetPosition().Z);
	ImGui::Text("Rotation: %.2f %.2f %.2f", 
		g_pObject->transform.GetRotation().X, 
		g_pObject->transform.GetRotation().Y, 
		g_pObject->transform.GetRotation().Z);
	ImGui::Text("Scale: %.2f %.2f %.2f", 
		g_pObject->transform.GetScale().X, 
		g_pObject->transform.GetScale().Y, 
		g_pObject->transform.GetScale().Z);
	ImGui::End();

	// === ImGuizmo 설정 ===
	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);

	// 조작 대상 변환 행렬
	Float4x4 mat = g_pObject->transform.GetWorldMatrixTranspose();
	DirectX::XMFLOAT4X4 worldMat;
	
	worldMat.m[0][0] = mat.r[0].X;
	worldMat.m[0][1] = mat.r[0].Y;
	worldMat.m[0][2] = mat.r[0].Z;
	worldMat.m[0][3] = mat.r[0].W;

	worldMat.m[1][0] = mat.r[1].X;
	worldMat.m[1][1] = mat.r[1].Y;
	worldMat.m[1][2] = mat.r[1].Z;
	worldMat.m[1][3] = mat.r[1].W;

	worldMat.m[2][0] = mat.r[2].X;
	worldMat.m[2][1] = mat.r[2].Y;
	worldMat.m[2][2] = mat.r[2].Z;
	worldMat.m[2][3] = mat.r[2].W;

	worldMat.m[3][0] = mat.r[3].X;
	worldMat.m[3][1] = mat.r[3].Y;
	worldMat.m[3][2] = mat.r[3].Z;
	worldMat.m[3][3] = mat.r[3].W;

	float matrix[16];
	memcpy(matrix, &worldMat, sizeof(float) * 16);

	static ImGuizmo::OPERATION currentOp = ImGuizmo::TRANSLATE;
	static ImGuizmo::MODE currentMode = ImGuizmo::WORLD;

	// === 키보드로 모드 전환 ===
	if (ImGui::IsKeyPressed(ImGuiKey_T)) currentOp = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) currentOp = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_S)) currentOp = ImGuizmo::SCALE;

	// === Gizmo 표시 영역 ===
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	// === 실제 기즈모 조작 ===
	ImGuizmo::Manipulate(
		(float*)&g_pCamera->view,
		(float*)&g_pCamera->proj,
		currentOp,
		currentMode,
		matrix
	);

	// === 수정 결과를 Transform으로 되돌림 ===
	if (ImGuizmo::IsUsing())
	{
		DirectX::XMMATRIX newMat = DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4*)matrix);
		MatrixToTransform(newMat, g_pObject->transform);
	}

	// === ImGui 렌더링 ===
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void RenderImgui(HWND hWnd)
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImguiExit(hWnd);

	ImguiPickingTransform();

	// ImguiDockingSpaceDemo();
	
	// ImguiDemo();

	// 렌더
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void RenderEnd()
{
	g_pSwapChain->Present(0, 0);
}

// ADD
void CleanupImgui()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// ADD
void Cleanup()
{
	CleanupImgui();

	if (g_pObject)
	{
		delete g_pObject;
		g_pObject = nullptr;
	}
	if (g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = nullptr;
	}
	if (g_pRasterizerState) g_pRasterizerState->Release();
	if (g_pAlphaBlendState) g_pAlphaBlendState->Release();
	if (g_pNormalMapShaderResourceView) g_pNormalMapShaderResourceView->Release();
	if (g_pSamplerLinear) g_pSamplerLinear->Release();
	if (g_pTextureResourceView) g_pTextureResourceView->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pCBTransform) g_pCBTransform->Release();
	if (g_pCBLight) g_pCBLight->Release();
	if (g_pCBMaterial) g_pCBMaterial->Release();
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

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// ADD
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

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
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(352);
#endif  // _DEBUG


	// ADD
	ImGui_ImplWin32_EnableDpiAwareness();
	float dpiScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
	
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

	
	DWORD dwStyle = WS_POPUP;
	RECT rc = { 0, 0, ResolutionWidth, ResolutionHeight };
	AdjustWindowRect(&rc, dwStyle, FALSE);
	
	// 윈도우 생성
	HWND hWnd = CreateWindowEx(
		0,
		L"WindowClass",
		L"Direct3D 11 윈도우",
		dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,   // 초기 위치
		rc.right - rc.left,             // 전체 윈도우 폭 (조정된 값)
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	ShowWindow(hWnd, SW_SHOWMAXIMIZED);

	if (FAILED(InitD3D(hWnd)))
	{
		Cleanup();
		return 1;
	}

	// ADD
	if (FAILED(InitImgui(hWnd, dpiScale)))
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

			// ADD
			RenderImgui(hWnd);

			RenderEnd();
		}
	}

	Cleanup();


#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif  // _DEBUG
	return (int)msg.wParam;
}


//#include "stdafx.h"
//using namespace DirectX;
//
//// === 구조체 ===
//struct Transform1
//{
//    XMFLOAT3 Position = { 0, 0, 0 };
//    XMFLOAT3 Rotation = { 0, 0, 0 };
//    XMFLOAT3 Scale = { 1, 1, 1 };
//};
//
//// === Transform ↔ Matrix 변환 ===
//XMMATRIX TransformToMatrix(const Transform1& t)
//{
//    return XMMatrixScaling(t.Scale.x, t.Scale.y, t.Scale.z) *
//        XMMatrixRotationRollPitchYaw(
//            XMConvertToRadians(t.Rotation.x),
//            XMConvertToRadians(t.Rotation.y),
//            XMConvertToRadians(t.Rotation.z)) *
//        XMMatrixTranslation(t.Position.x, t.Position.y, t.Position.z);
//}
//
//void MatrixToTransform(const XMMATRIX& m, Transform1& out)
//{
//    XMFLOAT4X4 mat;
//    XMStoreFloat4x4(&mat, m);
//    out.Position = { mat._41, mat._42, mat._43 };
//
//    out.Scale.x = sqrtf(mat._11 * mat._11 + mat._12 * mat._12 + mat._13 * mat._13);
//    out.Scale.y = sqrtf(mat._21 * mat._21 + mat._22 * mat._22 + mat._23 * mat._23);
//    out.Scale.z = sqrtf(mat._31 * mat._31 + mat._32 * mat._32 + mat._33 * mat._33);
//
//    out.Rotation.y = atan2f(-mat._13, sqrtf(mat._11 * mat._11 + mat._12 * mat._12));
//    out.Rotation.x = atan2f(mat._23, mat._33);
//    out.Rotation.z = atan2f(mat._12, mat._11);
//    out.Rotation.x = XMConvertToDegrees(out.Rotation.x);
//    out.Rotation.y = XMConvertToDegrees(out.Rotation.y);
//    out.Rotation.z = XMConvertToDegrees(out.Rotation.z);
//}
//
//// === 간단한 상수 버퍼 ===
//struct CBData
//{
//    XMMATRIX World;
//    XMMATRIX View;
//    XMMATRIX Proj;
//};
//
//// === 전역 ===
//Transform1 g_ObjectTransform;
//ImGuizmo::OPERATION g_CurrentOp = ImGuizmo::TRANSLATE;
//ImGuizmo::MODE g_CurrentMode = ImGuizmo::WORLD;
//
//// === 카메라 행렬 ===
//XMMATRIX g_View = XMMatrixLookAtLH(
//    XMVectorSet(0, 2, -5, 1),
//    XMVectorSet(0, 0, 0, 1),
//    XMVectorSet(0, 1, 0, 0));
//XMMATRIX g_Proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 16.0f / 9.0f, 0.1f, 100.0f);
//
//ID3D11Buffer* g_ConstantBuffer = nullptr;
//
//// === 기본 큐브 버텍스 정의 ===
//struct Vertex
//{
//    XMFLOAT3 pos;
//    XMFLOAT4 color;
//};
//
//ID3D11Buffer* g_VertexBuffer = nullptr;
//ID3D11Buffer* g_IndexBuffer = nullptr;
//
//// === 초기화: 간단한 큐브 만들기 ===
//void CreateCube(ID3D11Device* device)
//{
//    Vertex vertices[] = {
//        {{-1, -1, -1}, {1, 0, 0, 1}}, {{-1, +1, -1}, {0, 1, 0, 1}},
//        {{+1, +1, -1}, {0, 0, 1, 1}}, {{+1, -1, -1}, {1, 1, 0, 1}},
//        {{-1, -1, +1}, {1, 0, 1, 1}}, {{-1, +1, +1}, {0, 1, 1, 1}},
//        {{+1, +1, +1}, {1, 1, 1, 1}}, {{+1, -1, +1}, {0, 0, 0, 1}},
//    };
//
//    uint16_t indices[] = {
//        0,1,2, 0,2,3, // front
//        4,6,5, 4,7,6, // back
//        4,5,1, 4,1,0, // left
//        3,2,6, 3,6,7, // right
//        1,5,6, 1,6,2, // top
//        4,0,3, 4,3,7  // bottom
//    };
//
//    D3D11_BUFFER_DESC vbd{};
//    vbd.Usage = D3D11_USAGE_DEFAULT;
//    vbd.ByteWidth = sizeof(vertices);
//    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//
//    D3D11_SUBRESOURCE_DATA vinit{};
//    vinit.pSysMem = vertices;
//    device->CreateBuffer(&vbd, &vinit, &g_VertexBuffer);
//
//    D3D11_BUFFER_DESC ibd{};
//    ibd.Usage = D3D11_USAGE_DEFAULT;
//    ibd.ByteWidth = sizeof(indices);
//    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//
//    D3D11_SUBRESOURCE_DATA iinit{};
//    iinit.pSysMem = indices;
//    device->CreateBuffer(&ibd, &iinit, &g_IndexBuffer);
//
//    // Constant Buffer
//    D3D11_BUFFER_DESC cbd{};
//    cbd.Usage = D3D11_USAGE_DEFAULT;
//    cbd.ByteWidth = sizeof(CBData);
//    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//    device->CreateBuffer(&cbd, nullptr, &g_ConstantBuffer);
//}
//
//// === 렌더링 ===
//void RenderFrame(ID3D11DeviceContext* context)
//{
//    ImGui_ImplDX11_NewFrame();
//    ImGui_ImplWin32_NewFrame();
//    ImGui::NewFrame();
//    ImGuizmo::BeginFrame();
//
//    ImGui::Begin("Gizmo Controls");
//    if (ImGui::Button("Translate")) g_CurrentOp = ImGuizmo::TRANSLATE;
//    ImGui::SameLine();
//    if (ImGui::Button("Rotate")) g_CurrentOp = ImGuizmo::ROTATE;
//    ImGui::SameLine();
//    if (ImGui::Button("Scale")) g_CurrentOp = ImGuizmo::SCALE;
//    ImGui::End();
//
//    // === ImGuizmo 세팅 ===
//    ImGuiIO& io = ImGui::GetIO();
//    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
//
//    XMMATRIX world = TransformToMatrix(g_ObjectTransform);
//    XMFLOAT4X4 worldMat;
//    XMStoreFloat4x4(&worldMat, world);
//    float matrix[16];
//    memcpy(matrix, &worldMat, sizeof(matrix));
//
//    // === Gizmo 표시 및 조작 ===
//    ImGuizmo::Manipulate((float*)&g_View, (float*)&g_Proj, g_CurrentOp, g_CurrentMode, matrix);
//
//    if (ImGuizmo::IsUsing())
//    {
//        XMMATRIX newMat = XMLoadFloat4x4((XMFLOAT4X4*)matrix);
//        MatrixToTransform(newMat, g_ObjectTransform);
//    }
//
//    // === 실제 큐브 렌더링 ===
//    CBData cb;
//    cb.World = XMMatrixTranspose(TransformToMatrix(g_ObjectTransform));
//    cb.View = XMMatrixTranspose(g_View);
//    cb.Proj = XMMatrixTranspose(g_Proj);
//    context->UpdateSubresource(g_ConstantBuffer, 0, nullptr, &cb, 0, 0);
//
//    UINT stride = sizeof(Vertex);
//    UINT offset = 0;
//    context->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
//    context->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
//    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//    context->VSSetConstantBuffers(0, 1, &g_ConstantBuffer);
//
//    context->DrawIndexed(36, 0, 0);
//
//    // === ImGui 렌더 ===
//    ImGui::Render();
//    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
//}
