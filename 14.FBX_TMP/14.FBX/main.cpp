#include "stdafx.h"

#define ResolutionWidth 2560.0f
#define	ResolutionHeigh 1440.0f


// Init
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr; // Device Context
IDXGISwapChain* g_pSwapChain = nullptr; // 스왑 체인
ID3D11RenderTargetView* g_pRenderTargetView = nullptr; // 렌더 타켓 뷰
ID3D11DepthStencilView* g_pDepthStencilView = nullptr; // 깊이 스텐실 뷰


// Mesh
struct SimpleVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 normal;
	DirectX::XMFLOAT4 tangent;
	DirectX::XMFLOAT2 uv;
};

struct ConstantBuffer
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
	DirectX::XMFLOAT4 lightColor;
	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT3 spotPosition;
	float spotRange;
	DirectX::XMFLOAT3 spotDirection;
	float spotAngle;
};


struct MeshData
{
	std::vector<SimpleVertex> meshVertices;
	std::vector<WORD> meshIndices;

	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;
};

//struct MaterialData
//{
//	FbxDouble3 diffuseColor;
//	FbxDouble3 specularColor;
//	FbxDouble3 emissiveColor;
//
//	float shininess = 0.0f;
//	float roughness = 0.0f;
//	float metallic = 0.0f;
//	float opacity = 1.0f;
//
//	std::string diffuseTex;
//	std::string normalTex;
//	std::string specularTex;
//	std::string metallicTex;
//	std::string roughnessTex;
//	std::string emissiveTex;
//
//	FbxLayerElement::EMappingMode uvMappingMode = FbxLayerElement::eByPolygonVertex;
//};


MeshData* g_pMeshData = nullptr;
float g_fRotaionAngle = 0.0f;

// Render
ID3D11InputLayout* g_pInputLayout = nullptr;
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
ID3D11BlendState* g_pAlphaBlendState = nullptr;
ID3D11RasterizerState* g_pRasterizerState = nullptr;


namespace temp
{
	//FbxMesh* FindMesh(FbxNode* node)
//{
//	if (nullptr == node)
//	{
//		return nullptr;
//	}
//
//	FbxMesh* mesh = node->GetMesh();
//	if (nullptr != mesh)
//	{
//		return mesh;
//	}
//
//	for (int32_t n = 0; n < node->GetChildCount(); ++n)
//	{
//		FbxMesh* findMesh = FindMesh(node->GetChild(n));
//		if (nullptr != findMesh) {
//			return findMesh;
//		}
//	}
//	return nullptr;
//}
//
//bool LoadFbxMesh(const char* fileName, std::vector<SimpleVertex>* outVertices, std::vector<WORD>* outIndices)
//{
//	FbxManager* manager = FbxManager::Create();
//	if (nullptr == manager)
//	{
//		return false;
//	}
//
//	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
//	manager->SetIOSettings(ios);
//
//	FbxImporter* importer = FbxImporter::Create(manager, "fbxImporter");
//	bool status = importer->Initialize(fileName, -1, manager->GetIOSettings());
//	if (false == status)
//	{
//		importer->Destroy();
//		manager->Destroy();
//		return false;
//	}
//
//	FbxScene* scene = FbxScene::Create(manager, "fbxScene");
//	importer->Import(scene);
//	importer->Destroy();
//
//	FbxAxisSystem engineAxis(FbxAxisSystem::eZAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
//	engineAxis.ConvertScene(scene);
//
//	FbxGeometryConverter geomConv(manager);
//	geomConv.Triangulate(scene, true);
//
//	FbxNode* rootNode = scene->GetRootNode();
//	if (nullptr == rootNode)
//	{
//		manager->Destroy();
//		return false;
//	}
//
//	FbxMesh* foundMesh = FindMesh(rootNode);
//	if (nullptr == foundMesh)
//	{
//		manager->Destroy();
//		return false;
//	}
//
//	// GetControlPointsCount() -> 물리적인 Vertex 갯수.
//	int32_t ctrlPointCount = foundMesh->GetControlPointsCount();
//	outVertices->reserve(ctrlPointCount);
//
//	// 버텍스 수집
//	for (int32_t n = 0; n < ctrlPointCount; ++n)
//	{
//		FbxVector4 p = foundMesh->GetControlPointAt(n);
//		SimpleVertex v;
//		v.position = { (float)p[0], (float)p[1], (float)p[2] };
//		v.normal = { 0.f, 1.f, 0.f };
//		v.UV = { 0.f, 0.f };
//		outVertices->push_back(v);
//	}
//
//	// 인덱스 수집
//	// GetPolygonCount() -> 물리적인 면 갯수.
//	// GetPolygonSize() -> Polygon이 몇 개의 Vertex로 이루어져 있는지. ex) PrimitiveType이 삼각형이면 3.
//	// GetPolygonVertex() -> 특정 폴리곤의 특정 꼭짓점이 참조하는 Control Point 인덱스를 반환
//	// 파라미터_1 : 	몇 번째 폴리곤인가
//	// 파라미터_2 :	그 폴리곤 안에서 몇 번째 꼭짓점인가(0 ~GetPolygonSize(p) - 1)
//	int32_t polygonCount = foundMesh->GetPolygonCount();
//	for (int32_t n = 0; n < polygonCount; ++n)
//	{
//		int32_t polygonSize = foundMesh->GetPolygonSize(n);
//		for (int32_t j = 0; j < polygonSize; ++j)
//		{
//			int32_t ctrlIndex = foundMesh->GetPolygonVertex(n, j);
//			outIndices->push_back(ctrlIndex);
//		}
//	}
//
//	int32_t normalElementCount = foundMesh->GetElementNormalCount();
//	if (0 < normalElementCount)
//	{
//		FbxGeometryElementNormal* normalElement = foundMesh->GetElementNormal(0);
//		switch (normalElement->GetMappingMode())
//		{
//		case FbxGeometryElement::eByControlPoint:
//			switch (normalElement->GetReferenceMode())
//			{
//			case FbxGeometryElement::eDirect:
//			{
//				for (int32_t n = 0; n < ctrlPointCount; ++n)
//				{
//					FbxVector4 tmp = normalElement->GetDirectArray().GetAt(n);
//					(*outVertices)[n].normal = { (float)tmp[0], (float)tmp[1], (float)tmp[2] };
//				}
//			} break;
//			case FbxGeometryElement::eIndexToDirect:
//			{
//				for (int32_t n = 0; n < ctrlPointCount; ++n)
//				{
//					int index = normalElement->GetIndexArray().GetAt(n);
//					FbxVector4 tmp = normalElement->GetDirectArray().GetAt(index);
//					(*outVertices)[n].normal = { (float)tmp[0], (float)tmp[1], (float)tmp[2] };
//				}
//			} break;
//			default:
//				DEBUG_BREAK();
//			} break;
//		case FbxGeometryElement::eByPolygonVertex:
//			switch (normalElement->GetReferenceMode())
//			{
//			case FbxGeometryElement::eDirect:
//			{
//				DEBUG_BREAK();
//			} break;
//			case FbxGeometryElement::eIndexToDirect:
//			{
//				DEBUG_BREAK();
//			} break;
//			default:
//				DEBUG_BREAK();
//			} break;
//		}
//	}
//
//	// 탄젠트 정보
//	int32_t tangentElementCount = foundMesh->GetElementTangentCount();
//	if (0 < tangentElementCount)
//	{
//		FbxGeometryElementTangent* tangenElement = foundMesh->GetElementTangent(0);
//		switch (tangenElement->GetMappingMode())
//		{
//		case FbxGeometryElement::eByControlPoint:
//			switch (tangenElement->GetReferenceMode())
//			{
//			case FbxGeometryElement::eDirect:
//			{
//				for (int32_t n = 0; n < ctrlPointCount; ++n)
//				{
//					FbxVector4 tmp = tangenElement->GetDirectArray().GetAt(n);
//					(*outVertices)[n].tangent = { (float)tmp[0], (float)tmp[1], (float)tmp[2], 1.f };
//
//					DirectX::XMVECTOR nVec = DirectX::XMLoadFloat3(&(*outVertices)[n].normal);
//					DirectX::XMFLOAT3 floatTangent = DirectX::XMFLOAT3((*outVertices)[n].tangent.x, (*outVertices)[n].tangent.y, (*outVertices)[n].tangent.z);
//					DirectX::XMVECTOR tVec = DirectX::XMLoadFloat3(&floatTangent);
//					DirectX::XMVECTOR bVec = DirectX::XMVector3Cross(nVec, tVec);
//					DirectX::XMVECTOR c = DirectX::XMVector3Cross(tVec, bVec);
//					float dotVal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(c, nVec));
//					float handedness = (dotVal < 0.0f) ? -1.0f : 1.0f;
//					(*outVertices)[n].tangent.z = handedness;
//				}
//			}
//			break;
//			case FbxGeometryElement::eIndexToDirect:
//			{
//				for (int32_t n = 0; n < ctrlPointCount; ++n)
//				{
//					int32_t tangentIndex = tangenElement->GetIndexArray().GetAt(n);
//					FbxVector4 tmp = tangenElement->GetDirectArray().GetAt(tangentIndex);
//					(*outVertices)[n].tangent = { (float)tmp[0], (float)tmp[1], (float)tmp[2], 1.f };
//
//					DirectX::XMVECTOR nVec = DirectX::XMLoadFloat3(&(*outVertices)[n].normal);
//					DirectX::XMFLOAT3 floatTangent = DirectX::XMFLOAT3((*outVertices)[n].tangent.x, (*outVertices)[n].tangent.y, (*outVertices)[n].tangent.z);
//					DirectX::XMVECTOR tVec = DirectX::XMLoadFloat3(&floatTangent);
//					DirectX::XMVECTOR bVec = DirectX::XMVector3Cross(nVec, tVec);
//					DirectX::XMVECTOR c = DirectX::XMVector3Cross(tVec, bVec);
//					float dotVal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(c, nVec));
//					float handedness = (dotVal < 0.0f) ? -1.0f : 1.0f;
//					(*outVertices)[n].tangent.z = handedness;
//				}
//			}
//			break;
//			default:
//				DEBUG_BREAK();
//			}
//			break;
//		case FbxGeometryElement::eByPolygonVertex:
//			switch (tangenElement->GetReferenceMode())
//			{
//			case FbxGeometryElement::eDirect:
//			{
//				DEBUG_BREAK();
//			}
//			break;
//			case FbxGeometryElement::eIndexToDirect:
//			{
//				DEBUG_BREAK();
//			}
//			break;
//			default:
//				DEBUG_BREAK();
//			}
//			break;
//		}
//	}
//	else
//	{
//		// 탄젠트가 없을 경우.
//		for (int32_t n = 0; n < ctrlPointCount; ++n)
//		{
//			const DirectX::XMFLOAT3& normalVec = (*outVertices)[n].normal;
//			DirectX::XMFLOAT3 up(0.0f, 0.0f, 1.0f);
//			float dotUp = fabsf(normalVec.x * up.x + normalVec.y * up.y + normalVec.z * up.z);
//			if (dotUp > 0.98f)
//			{
//				up = DirectX::XMFLOAT3(0.f, 1.0f, 0.0f);
//			}
//
//			DirectX::XMVECTOR normalV = DirectX::XMLoadFloat3(&normalVec);
//			DirectX::XMVECTOR upV = DirectX::XMLoadFloat3(&up);
//			DirectX::XMVECTOR tVec = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(normalV, upV));
//			DirectX::XMFLOAT3 t3;
//			DirectX::XMStoreFloat3(&t3, tVec);
//
//			(*outVertices)[n].tangent = { t3.x, t3.y, t3.z, 1.f };
//
//			DirectX::XMVECTOR bVec = DirectX::XMVector3Cross(normalV, tVec);
//			DirectX::XMVECTOR c = DirectX::XMVector3Cross(tVec, bVec);
//			float dotVal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(c, normalV));
//			float handedness = (dotVal < 0.0f) ? -1.0f : 1.0f;
//			(*outVertices)[n].tangent.z = handedness;
//		}
//	}
//
//
//
//	int32_t uvElementCount = foundMesh->GetElementUVCount();
//	if (0 < uvElementCount)
//	{
//		FbxGeometryElementUV* uvElement = foundMesh->GetElementUV(0);
//		switch (uvElement->GetMappingMode())
//		{
//		case FbxGeometryElement::eByControlPoint:
//			switch (uvElement->GetReferenceMode())
//			{
//			case FbxGeometryElement::eDirect:
//			{
//				for (int32_t n = 0; n < ctrlPointCount; ++n)
//				{
//					FbxVector2 uv = uvElement->GetDirectArray().GetAt(n);
//					(*outVertices)[n].UV = { (float)uv[0], (float)uv[1] };
//				}
//			}
//			break;
//			case FbxGeometryElement::eIndexToDirect:
//			{
//				for (int32_t n = 0; n < ctrlPointCount; ++n)
//				{
//					int32_t uvIndex = uvIndex = uvElement->GetIndexArray().GetAt(n);
//					FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
//					(*outVertices)[n].UV = { (float)uv[0], (float)uv[1] };
//				}
//			}
//			break;
//			default:
//				DEBUG_BREAK();
//			}
//			break;
//		case FbxGeometryElement::eByPolygonVertex:
//			switch (uvElement->GetReferenceMode())
//			{
//			case FbxGeometryElement::eDirect:
//			{
//				DEBUG_BREAK();
//			}
//			break;
//			case FbxGeometryElement::eIndexToDirect:
//			{
//				DEBUG_BREAK();
//			}
//			break;
//			default:
//				DEBUG_BREAK();
//			}
//			break;
//		}
//	}
//
//	manager->Destroy();
//	return true;
//}
//
//void ProcessMaterial(FbxSurfaceMaterial* mat, MaterialData& out)
//{
//	if (!mat) return;
//
//	// ===========================
//	// (1) 기본 색상 (Diffuse / Specular / Emissive)
//	// ===========================
//	auto getColor3 = [&](const char* name)->FbxDouble3 {
//		FbxProperty prop = mat->FindProperty(name);
//		if (prop.IsValid())
//			return prop.Get<FbxDouble3>();
//		return FbxDouble3(1.0, 1.0, 1.0);
//		};
//
//	out.diffuseColor = getColor3(FbxSurfaceMaterial::sDiffuse);
//	out.specularColor = getColor3(FbxSurfaceMaterial::sSpecular);
//	out.emissiveColor = getColor3(FbxSurfaceMaterial::sEmissive);
//
//
//	// ===========================
//	// (2) Shininess / Roughness / Metallic / Opacity
//	// ===========================
//	auto getFloat = [&](const char* name, float def)->float {
//		FbxProperty prop = mat->FindProperty(name);
//		if (prop.IsValid())
//			return (float)prop.Get<FbxDouble>();
//		return def;
//		};
//
//	out.shininess = getFloat(FbxSurfaceMaterial::sShininess, 0.0f);
//	out.opacity = getFloat(FbxSurfaceMaterial::sTransparencyFactor, 1.0f);
//
//	// Roughness / Metallic 은 PBR 특성 → 최신 FBX에서 사용
//	out.roughness = getFloat("Roughness", 0.5f);
//	out.metallic = getFloat("Metalness", 0.0f);
//
//
//	// ===========================
//	// (3) Texture 파일 경로 얻기
//	// ===========================
//	auto getTexture = [&](const char* propName)->std::string {
//		FbxProperty prop = mat->FindProperty(propName);
//		if (!prop.IsValid()) return "";
//
//		int texCount = prop.GetSrcObjectCount<FbxFileTexture>();
//		if (texCount > 0)
//		{
//			FbxFileTexture* tex = prop.GetSrcObject<FbxFileTexture>(0);
//			if (tex)
//				return tex->GetFileName();  // 절대경로 or 상대경로
//		}
//		return "";
//		};
//
//	out.diffuseTex = getTexture(FbxSurfaceMaterial::sDiffuse);
//	out.specularTex = getTexture(FbxSurfaceMaterial::sSpecular);
//	out.normalTex = getTexture(FbxSurfaceMaterial::sNormalMap);
//	out.emissiveTex = getTexture(FbxSurfaceMaterial::sEmissive);
//	out.metallicTex = getTexture("Metalness");
//	out.roughnessTex = getTexture("Roughness");
//}
//
//HRESULT InitMesh()
//{
//	//LoadFbxMesh("..\\..\\Resource\\fbx\\Mannequin.FBX", &meshVertices, &meshIndices);
//	//LoadFbxMesh("..\\..\\Resource\\fbx\\AnimMan.FBX", &meshVertices, &meshIndices);
//	//LoadFbxMesh("..\\..\\Resource\\fbx\\SK_Troll.FBX", &meshVertices, &meshIndices);
//	//LoadFbxMesh("..\\..\\Resource\\fbx\\SK_Barbarian_Body.FBX", &meshVertices, &meshIndices);
//	LoadFbxMesh("..\\..\\Resource\\fbx\\JUMPER_MESH.FBX", &g_pMeshData->meshVertices, &g_pMeshData->meshIndices);
//
//
//
//	D3D11_BUFFER_DESC bd;
//	memset(&bd, 0x00, sizeof(D3D11_BUFFER_DESC));
//	bd.Usage = D3D11_USAGE_DEFAULT;
//	bd.ByteWidth = (UINT)(sizeof(SimpleVertex) * g_pMeshData->meshVertices.size());
//	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	bd.CPUAccessFlags = 0;
//
//	D3D11_SUBRESOURCE_DATA InitData;
//	memset(&InitData, 0x00, sizeof(D3D11_SUBRESOURCE_DATA));
//	InitData.pSysMem = g_pMeshData->meshVertices.data();
//
//	HRESULT hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pMeshData->pVertexBuffer);
//	if (FAILED(hr))
//	{
//		return hr;
//	}
//
//	// 인덱스 버퍼
//	bd.Usage = D3D11_USAGE_DEFAULT;
//	bd.ByteWidth = (UINT)(sizeof(WORD) * g_pMeshData->meshIndices.size());
//	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	bd.CPUAccessFlags = 0;
//
//	InitData.pSysMem = g_pMeshData->meshIndices.data();
//	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pMeshData->pIndexBuffer);
//	if (FAILED(hr))
//	{
//		return hr;
//	}
//
//	return S_OK;
//}
//
}

FbxMesh* ProcessMesh(FbxNode* node)
{
	if (nullptr == node)
	{
		return nullptr;
	}

	FbxMesh* mesh = node->GetMesh();
	if (nullptr != mesh)
	{
		return mesh;
	}

	for (int32_t n = 0; n < node->GetChildCount(); ++n)
	{
		FbxMesh* findMesh = ProcessMesh(node->GetChild(n));
		if (nullptr != findMesh) {
			return findMesh;
		}
	}
	return nullptr;
}


void ExtractMeshData(FbxMesh* mesh)
{
	int polygonCount = mesh->GetPolygonCount();
	int ctrlPointCount = mesh->GetControlPointsCount();
	FbxVector4* ctrlPoints = mesh->GetControlPoints();

	// Normal / Tangent / Binormal / UV Layer Elements
	FbxGeometryElementNormal* normalElem = mesh->GetElementNormal(0);
	FbxGeometryElementTangent* tangentElem = mesh->GetElementTangent(0);
	FbxGeometryElementUV* uvElem = mesh->GetElementUV(0);

	g_pMeshData->meshVertices.resize(ctrlPointCount);
	int vertexCounter = 0; // polygonVertexIndex

	for (int polyIdx = 0; polyIdx < polygonCount; polyIdx++)
	{
		int polySize = mesh->GetPolygonSize(polyIdx);
		if (3 != polySize)
		{
			DEBUG_BREAK();
			return;
		}

		for (int posInPoly = 0; posInPoly < polySize; posInPoly++)
		{
			int ctrlPointIndex = mesh->GetPolygonVertex(polyIdx, posInPoly);
			//// -------------------------
			//// ① Vertex(Position), Index
			//// -------------------------
			FbxVector4 vertexPos = ctrlPoints[ctrlPointIndex];
			g_pMeshData->meshVertices[ctrlPointIndex].position = { (float)vertexPos[0], (float)vertexPos[1], (float)vertexPos[2] };
			g_pMeshData->meshIndices.push_back((WORD)ctrlPointIndex);


			// -------------------------
			// ② Normal
			// -------------------------
			FbxVector4 normal(0, 0, 0, 0);
			int32_t normalElementCount = mesh->GetElementNormalCount();
			if (normalElem && 0 < normalElementCount)
			{
				if (normalElem->GetMappingMode() == FbxGeometryElement::eByControlPoint)
				{
					int idx = (normalElem->GetReferenceMode() == FbxGeometryElement::eDirect)
						? ctrlPointIndex : normalElem->GetIndexArray().GetAt(ctrlPointIndex);

					normal = normalElem->GetDirectArray().GetAt(idx);
					g_pMeshData->meshVertices[ctrlPointIndex].normal =
					{
						(float)normal[0],
						(float)normal[1],
						(float)normal[2],
						(float)normal[3]
					};
				}
				else if (normalElem->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					int idx = (normalElem->GetReferenceMode() == FbxGeometryElement::eDirect)
						? vertexCounter : normalElem->GetIndexArray().GetAt(vertexCounter);

					normal = normalElem->GetDirectArray().GetAt(idx);
					g_pMeshData->meshVertices[ctrlPointIndex].normal =
					{
						(float)normal[0],
						(float)normal[1],
						(float)normal[2],
						(float)normal[3]
					};
				}
			}

			// -------------------------
			// ③ Tangent
			// -------------------------
			FbxVector4 tangent(0, 0, 0, 0);
			int32_t tangentElementCount = mesh->GetElementTangentCount();
			if (tangentElem && 0 < tangentElementCount)
			{
				if (tangentElem->GetMappingMode() == FbxGeometryElement::eByControlPoint)
				{
					int idx = (tangentElem->GetReferenceMode() == FbxGeometryElement::eDirect)
						? vertexCounter : tangentElem->GetIndexArray().GetAt(vertexCounter);

					tangent = tangentElem->GetDirectArray().GetAt(idx);

					// 주의! Tangent.W 값이 -1. 혹은 1 이 아니면 Tangent Sign을 직접 구해야 함.
					if (!(tangent[3] == -1 || tangent[3] == 1))
					{
						DEBUG_BREAK();
					}

					g_pMeshData->meshVertices[ctrlPointIndex].tangent =
					{
						(float)tangent[0],
						(float)tangent[1],
						(float)tangent[2],
						(float)tangent[3]
					};
				}
				else if (tangentElem->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					// 아직 준비 안됨.
					DEBUG_BREAK();
				}
			}
			else
			{
				// Tanget가 없는 경우 직접 계산.
				DirectX::XMFLOAT4 tmpNormal4 = g_pMeshData->meshVertices[ctrlPointIndex].normal;
				DirectX::XMFLOAT3 tmpNormal3 = { tmpNormal4.x, tmpNormal4.y, tmpNormal4.z };
				DirectX::XMFLOAT3 up(0.0f, 0.0f, 1.0f);
				float dotUp = fabsf(tmpNormal3.x * up.x + tmpNormal3.y * up.y + tmpNormal3.z * up.z);
				if (dotUp > 0.98f)
				{
					up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
				}

				DirectX::XMVECTOR normalV = DirectX::XMLoadFloat3(&tmpNormal3);
				DirectX::XMVECTOR upV = DirectX::XMLoadFloat3(&up);
				DirectX::XMVECTOR tVec = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(normalV, upV));
				DirectX::XMFLOAT3 t3;
				DirectX::XMStoreFloat3(&t3, tVec);
				g_pMeshData->meshVertices[ctrlPointIndex].tangent = { t3.x, t3.y, t3.z, 1.f };

				DirectX::XMVECTOR bVec = DirectX::XMVector3Cross(normalV, tVec);
				DirectX::XMVECTOR c = DirectX::XMVector3Cross(tVec, bVec);
				float dotVal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(c, normalV));
				float handedness = (dotVal < 0.0f) ? -1.0f : 1.0f;
				g_pMeshData->meshVertices[ctrlPointIndex].tangent.z = handedness;
			}

			// -------------------------
			// ④ Binormal	// 직접 구할 필요는 없음. GPU에서 계산.
			// -------------------------
			//FbxVector4 binormal(0, 0, 0, 0);
			//int32_t binormalElementCount = mesh->GetElementTangentCount();
			//if (binormalElem && 0 < binormalElementCount)
			//{
			//	if (binormalElem->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			//	{
			//		int idx = (binormalElem->GetReferenceMode() == FbxGeometryElement::eDirect)
			//			? vertexCounter : binormalElem->GetIndexArray().GetAt(vertexCounter);

			//		binormal = binormalElem->GetDirectArray().GetAt(idx);
			//		g_pMeshData->meshVertices[ctrlPointIndex].binormal =
			//		{
			//			(float)binormal[0],
			//			(float)binormal[1],
			//			(float)binormal[2],
			//			(float)binormal[3]
			//		};
			//	}
			//	else if (binormalElem->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			//	{
			//		// 아직 준비 안됨.
			//		DEBUG_BREAK();
			//	}
			//}
			//else
			//{
			//	// Binormal이 없을경우 직접 구해야 함.

			// -------------------------
			// ⑤ UV
			// -------------------------
			int32_t uvElementCount = mesh->GetElementUVCount();
			if (uvElem && 0 < uvElementCount)
			{
				if (uvElem->GetMappingMode() == FbxGeometryElement::eByControlPoint)
				{
					int idx = (uvElem->GetReferenceMode() == FbxGeometryElement::eDirect)
						? vertexCounter : uvElem->GetIndexArray().GetAt(vertexCounter);

					FbxVector2 uv = uvElem->GetDirectArray().GetAt(idx);
					g_pMeshData->meshVertices[ctrlPointIndex].uv = { (float)uv[0], (float)uv[1] };
				}
				else if (uvElem->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					// 아직 준비 안됨.
					DEBUG_BREAK();
				}
			}
			vertexCounter++;
		}
	}
}


bool LoadMesh(const char* fileName)
{
	bool isSuccess = false;

	FbxManager* manager = nullptr;
	FbxIOSettings* ioSetting = nullptr;
	FbxImporter* importer = nullptr;
	FbxScene* scene = nullptr;
	do
	{
		manager = FbxManager::Create();
		if (nullptr == manager)
		{
			DEBUG_BREAK();
			break;
		}

		ioSetting = FbxIOSettings::Create(manager, IOSROOT);
		manager->SetIOSettings(ioSetting);

		importer = FbxImporter::Create(manager, "fbxImporter");
		if (!importer->Initialize(fileName, -1, manager->GetIOSettings()))
		{
			DEBUG_BREAK();
			break;
		}

		FbxScene* scene = FbxScene::Create(manager, "fbxScene");
		importer->Import(scene);

		FbxAxisSystem engineAxis(FbxAxisSystem::eZAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
		engineAxis.ConvertScene(scene);

		FbxGeometryConverter geomConv(manager);
		geomConv.Triangulate(scene, true);

		FbxNode* rootNode = scene->GetRootNode();
		if (nullptr == rootNode)
		{
			DEBUG_BREAK();
			break;
		}

		FbxMesh* foundMesh = ProcessMesh(rootNode);
		if (nullptr == foundMesh)
		{
			DEBUG_BREAK();
			break;
		}

		ExtractMeshData(foundMesh);

		//int32_t ctrlPointCount = foundMesh->GetControlPointsCount();
		//g_pMeshData->meshVertices.reserve(ctrlPointCount);
		//
		//// 버텍스 수집
		//for (int32_t n = 0; n < ctrlPointCount; ++n)
		//{
		//	FbxVector4 p = foundMesh->GetControlPointAt(n);
		//	SimpleVertex v;
		//	v.position = { (float)p[0], (float)p[1], (float)p[2] };
		//	//v.normal = { 0.f, 1.f, 0.f };
		//	//v.UV = { 0.f, 0.f };
		//	g_pMeshData->meshVertices.push_back(v);
		//}

		//// 인덱스 수집.
		//int32_t polygonCount = foundMesh->GetPolygonCount();
		//for (int32_t n = 0; n < polygonCount; ++n)
		//{
		//	int32_t polygonSize = foundMesh->GetPolygonSize(n);
		//	if (polygonSize != 3)
		//	{
		//		DEBUG_BREAK();
		//		break;
		//	}
		//	for (int32_t j = 0; j < polygonSize; ++j)
		//	{
		//		int32_t ctrlIndex = foundMesh->GetPolygonVertex(n, j);
		//		g_pMeshData->meshIndices.push_back(ctrlIndex);
		//	}
		//}

		isSuccess = true;
		break;
	} while (false);

	if (nullptr != scene)
	{
		scene->Destroy();
		scene = nullptr;
	}
	if (nullptr != importer)
	{
		importer->Destroy();
		importer = nullptr;
	}
	if (nullptr != ioSetting)
	{
		ioSetting->Destroy();
		ioSetting = nullptr;
	}
	if (nullptr != manager)
	{
		manager->Destroy();
		manager = nullptr;
	}

	return isSuccess;
}

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

	return S_OK;
}

HRESULT InitInputLayout(ID3DBlob* pVSBlob)
{
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0}
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
	//HRESULT hr = LoadWhiteTexture(&g_pTextureResourceView, 0xFFFFFFFF);
	//if (FAILED(hr))
	//{
	//	DEBUG_BREAK();
	//	return hr;
	//}

	//hr = LoadWhiteTexture(&g_pNormalMapShaderResourceView, 0xFF8080FF);
	//if (FAILED(hr))
	//{
	//	DEBUG_BREAK();
	//	return hr;
	//}

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
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;         // 선명하게 보여야할때 -> 16개
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // 반복
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
	//rasDesc.CullMode = D3D11_CULL_FRONT;
	rasDesc.CullMode = D3D11_CULL_BACK;
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

	// 월드, 뷰, 프로젝션 행렬 설정
	// world는 오브젝트마다 고유의 값이며, 각각의 오브젝트의 Transform 을 적용해야함.
	DirectX::XMMATRIX world = scale * rotation * position;
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(-50.0f, 0.0f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f), DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, ResolutionWidth / ResolutionHeigh, 0.01f, 1000.0f);

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
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pMeshData->pVertexBuffer, &stride, &offset);

	// 인덱스 버퍼 설정
	g_pImmediateContext->IASetIndexBuffer(g_pMeshData->pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

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
	// ( SrcColor * blendFactor ) + (desColor * (1 - blendFactor))
	// + blendFactor를 조절하여 Fade In Out 기능을 구현할수 있다.
	// + 또는 색 강조도가능.
	// + 하지만 BlendFactor를 사용하면 Alpha 기능은 사용하지 못함.=
	//  BlendFactor 사용시 설정.
	// + blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
	// + blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR; // 1 - BlendFactor  // 렌더타켓

	g_pImmediateContext->OMSetBlendState(g_pAlphaBlendState, blendFactor, 0xffffffff);
}

HRESULT CreateBoxMesh()
{
	//g_pMeshData->meshVertices =
	//{
	//	// 아래 (-Z) - Normal(0,0,-1,0), Tangent(-1,0,0,1) -> Binormal(0, -1, 0, 1)
	//{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,-1,0), DirectX::XMFLOAT4(-1,0,0,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(0,1)},
	//{DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,-1,0), DirectX::XMFLOAT4(-1,0,0,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(0,0)},
	//{DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,-1,0), DirectX::XMFLOAT4(-1,0,0,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(1,0)},
	//{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,-1,0), DirectX::XMFLOAT4(-1,0,0,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(1,1)},

	//// 위 (+Z) - Normal(0,0,1,0), Tangent(1,0,0,1) -> Binormal(0, 1, 0, 1)
	//{DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,1,0),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT2(1,1)},
	//{DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,1,0),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT2(0,1)},
	//{DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,1,0),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT2(0,0)},
	//{DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,1,0),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT2(1,0)},

	//// 앞 (-X) - Normal(-1,0,0,0), Tangent(0,0,1,1) -> Binormal(0, -1, 0, 1)
	//{DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT4(-1,0,0,0),DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(0,1)},
	//{DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT4(-1,0,0,0),DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(0,0)},
	//{DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT4(-1,0,0,0),DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(1,0)},
	//{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT4(-1,0,0,0),DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(1,1)},

	//// 뒤 (+X) - Normal(1,0,0,0), Tangent(0,0,-1,1) -> Binormal(0, -1, 0, 1)
	//{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT4(1,0,0,0),DirectX::XMFLOAT4(0,0,-1,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(0,1)},
	//{DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT4(1,0,0,0),DirectX::XMFLOAT4(0,0,-1,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(0,0)},
	//{DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT4(1,0,0,0),DirectX::XMFLOAT4(0,0,-1,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(1,0)},
	//{DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0,1,0,1), DirectX::XMFLOAT4(1,0,0,0),DirectX::XMFLOAT4(0,0,-1,1), DirectX::XMFLOAT4(0,-1,0,1), DirectX::XMFLOAT2(1,1)},

	//// 오른쪽 (+Y) - Normal(0,1,0,0), Tangent(1,0,0,1) -> Binormal(0, 0, 1, 1)
	//{DirectX::XMFLOAT3(-0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,1,0,0),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT2(0,1)},
	//{DirectX::XMFLOAT3(-0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,1,0,0),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT2(0,0)},
	//{DirectX::XMFLOAT3(0.5f,  0.5f,  0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,1,0,0),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT2(1,0)},
	//{DirectX::XMFLOAT3(0.5f,  0.5f, -0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,1,0,0),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT2(1,1)},

	//// 왼쪽 (-Y) - Normal(0,-1,0,0), Tangent(-1,0,1,1) -> Binormal(1, 0, 0, 1)
	//{DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,-1,0,0),DirectX::XMFLOAT4(-1,0,1,1),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT2(1,1)},
	//{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,-1,0,0),DirectX::XMFLOAT4(-1,0,1,1),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT2(0,1)},
	//{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,-1,0,0) ,DirectX::XMFLOAT4(-1,0,1,1),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT2(0,0)},
	//{DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT4(0,0,1,1), DirectX::XMFLOAT4(0,-1,0,0) ,DirectX::XMFLOAT4(-1,0,1,1),DirectX::XMFLOAT4(1,0,0,1), DirectX::XMFLOAT2(1,0)},
	//};

	//g_pMeshData->meshIndices =
	//{
	//0,1,2, 0,2,3,       // Front
	//4,5,6, 4,6,7,       // Back
	//8,9,10, 8,10,11,    // Left
	//12,13,14, 12,14,15, // Right
	//16,17,18, 16,18,19, // Top
	//20,21,22, 20,22,23  // Bottom
	//};

	D3D11_BUFFER_DESC bd;
	memset(&bd, 0x00, sizeof(D3D11_BUFFER_DESC));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = (UINT)(sizeof(SimpleVertex) * g_pMeshData->meshVertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	memset(&InitData, 0x00, sizeof(D3D11_SUBRESOURCE_DATA));
	InitData.pSysMem = g_pMeshData->meshVertices.data();

	HRESULT hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pMeshData->pVertexBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	// 인덱스 버퍼
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = (UINT)(sizeof(WORD) * g_pMeshData->meshIndices.size());
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	InitData.pSysMem = g_pMeshData->meshIndices.data();
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pMeshData->pIndexBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

void BeginPlay()
{
	InitDepthStencilBuffer();

	g_pMeshData = new MeshData;

	//LoadMesh("..\\..\\Resource\\fbx\\Mannequin.FBX");
	//LoadMesh("..\\..\\Resource\\fbx\\AnimMan.FBX");
	LoadMesh("..\\..\\Resource\\fbx\\SK_Troll.FBX");
	//LoadMesh("..\\..\\Resource\\fbx\\SK_Barbarian_Body.FBX");
	//LoadMesh("..\\..\\Resource\\fbx\\JUMPER_MESH.FBX");

	CreateBoxMesh();

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

void RenderBegin()
{
	float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Render()
{
	g_fRotaionAngle += 0.05f;
	Transform tf1;
	tf1.SetScale({ 0.1f, 0.1f, 0.1f });
	tf1.SetRotation({ 0.0f, 0.0f, g_fRotaionAngle });
	tf1.SetPosition({ 0.0f, -10.0f, -5.0f });
	UpdateConstantResource(tf1);
	g_pImmediateContext->DrawIndexed(g_pMeshData->meshIndices.size(), 0, 0);


	Transform tf2;
	tf2.SetScale({ 0.1f, 0.1f, 0.1f });
	tf2.SetRotation({ 0.0f, 0.0f, -g_fRotaionAngle });
	tf2.SetPosition({ 0.0f, 10.0f, -5.0f });
	UpdateConstantResource(tf2);
	g_pImmediateContext->DrawIndexed(g_pMeshData->meshIndices.size(), 0, 0);

}

void RenderEnd()
{
	g_pSwapChain->Present(0, 0);
}

void Cleanup()
{
	if (g_pMeshData)
	{
		if (g_pMeshData->pVertexBuffer)
		{
			g_pMeshData->pVertexBuffer->Release();
			g_pMeshData->pVertexBuffer = nullptr;
		}
		if (g_pMeshData->pIndexBuffer)
		{
			g_pMeshData->pIndexBuffer->Release();
			g_pMeshData->pIndexBuffer = nullptr;
		}
		delete g_pMeshData;
		g_pMeshData = nullptr;
	}

	if (g_pRasterizerState) g_pRasterizerState->Release();
	if (g_pAlphaBlendState) g_pAlphaBlendState->Release();
	if (g_pNormalMapShaderResourceView) g_pNormalMapShaderResourceView->Release();
	if (g_pSamplerLinear) g_pSamplerLinear->Release();
	if (g_pTextureResourceView) g_pTextureResourceView->Release();
	if (g_pPixelShader) g_pPixelShader->Release();
	if (g_pVertexShader) g_pVertexShader->Release();
	if (g_pConstantBuffer) g_pConstantBuffer->Release();
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