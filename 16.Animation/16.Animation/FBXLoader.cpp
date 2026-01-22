#include "stdafx.h"
#include "FBXLoader.h"

FBXLoader::FBXLoader()
	: pManager_(nullptr)
	, pIOSetting_(nullptr)
	, pImporter_(nullptr)
	, pScene_(nullptr)
	, sceneAxisInfo_()
{
}

FBXLoader::~FBXLoader()
{
	CleanUp();
}


bool FBXLoader::Init(const std::string& file)
{
	CleanUp();

	pManager_ = fbxsdk::FbxManager::Create();
	if (nullptr == pManager_)
	{
		DEBUG_BREAK();
		return false;
	}

	pIOSetting_ = fbxsdk::FbxIOSettings::Create(pManager_, IOSROOT);
	pManager_->SetIOSettings(pIOSetting_);

	pImporter_ = fbxsdk::FbxImporter::Create(pManager_, "fbxImporter");
	bool status = pImporter_->Initialize(file.c_str(), -1, pManager_->GetIOSettings());
	if (false == status)
	{
		DEBUG_BREAK();
		return false;
	}

	pScene_ = fbxsdk::FbxScene::Create(pManager_, "fbxScene");
	pImporter_->Import(pScene_);

	//fbxsdk::FbxAxisSystem engineAxis
	//(
	//	fbxsdk::FbxAxisSystem::eZAxis, fbxsdk::FbxAxisSystem::eParityEven, fbxsdk::FbxAxisSystem::eLeftHanded
	//);
	//engineAxis.ConvertScene(pScene_);

	sceneAxisInfo_ = GetSceneAxisInfo(pScene_);


	fbxsdk::FbxGeometryConverter geomConv(pManager_);
	geomConv.Triangulate(pScene_, true);

	return true;
}

bool FBXLoader::LoadMesh(FBXMesh* pOutMesh, const std::string& file)
{
	if (!Init(file))
	{
		DEBUG_BREAK();
		return false;
	}

	fbxsdk::FbxNode* pRootNode = pScene_->GetRootNode();
	int totalMeshCount = CountMeshes(pRootNode);
	if (1 != totalMeshCount)
	{
		DEBUG_BREAK();
	}

	fbxsdk::FbxMesh* pMesh = FindMesh(pRootNode);
	if (nullptr == pMesh)
	{
		DEBUG_BREAK();
		return false;
	}

	ExtractMeshData(pOutMesh->pData_, pMesh);
	ExtractMeshColor(pOutMesh->pData_, pMesh);
	ExtractMeshNormal(pOutMesh->pData_, pMesh);
	ExtractMeshTangent(pOutMesh->pData_, pMesh);
	ExtractMeshUV(pOutMesh->pData_, pMesh);

	FindBones(pRootNode, -1, pOutMesh);
	FindSkinWeight(pOutMesh, pMesh, pOutMesh->skinData_, pOutMesh->boneMap_);
	SkinDataToVertexData(pOutMesh);

	sceneAxisInfo_;
	int k = 10;

	return true;
}


fbxsdk::FbxMesh* FBXLoader::FindMesh(fbxsdk::FbxNode* node)
{
	if (nullptr == node)
	{
		return nullptr;
	}

	fbxsdk::FbxMesh* mesh = node->GetMesh();
	if (nullptr != mesh)
	{
		return mesh;
	}

	for (int32_t n = 0; n < node->GetChildCount(); ++n)
	{
		fbxsdk::FbxMesh* findMesh = FindMesh(node->GetChild(n));
		if (nullptr != findMesh)
		{
			return findMesh;
		}
	}
	return nullptr;
}

void FBXLoader::ExtractMeshData(MeshData* pMeshData, fbxsdk::FbxMesh* pMesh)
{
	if (nullptr == pMesh)
	{
		DEBUG_BREAK();
		return;
	}

	pMeshData->meshVertices.clear();
	pMeshData->meshIndices.clear();

	int32_t ctrlPointCount = pMesh->GetControlPointsCount();
	pMeshData->meshVertices.reserve(ctrlPointCount);

	// 버텍스 수집
	for (int32_t n = 0; n < ctrlPointCount; ++n)
	{
		FbxVector4 p = pMesh->GetControlPointAt(n);
		SimpleVertex v;
		v.position = { (float)p[0], (float)p[1], (float)p[2] };
		pMeshData->meshVertices.push_back(v);
	}

	// 인덱스 수집
	int32_t polygonCount = pMesh->GetPolygonCount();
	for (int32_t n = 0; n < polygonCount; ++n)
	{
		int32_t polygonSize = pMesh->GetPolygonSize(n);
		for (int32_t j = 0; j < polygonSize; ++j)
		{
			int32_t ctrlIndex = pMesh->GetPolygonVertex(n, j);
			pMeshData->meshIndices.push_back(ctrlIndex);
		}
	}
}

void FBXLoader::ExtractMeshColor(MeshData* pMeshData, fbxsdk::FbxMesh* pMesh)
{
	int32_t ctrlPointCount = pMesh->GetControlPointsCount();

	// Vertex Color 수집.
	FbxGeometryElementVertexColor* colorElem = pMesh->GetElementVertexColor(0);
	if (colorElem)
	{
		auto mapMode = colorElem->GetMappingMode();
		auto refMode = colorElem->GetReferenceMode();
		if (mapMode == FbxGeometryElement::eByControlPoint)
		{
			for (int i = 0; i < ctrlPointCount; i++)
			{
				int index = (refMode == FbxGeometryElement::eDirect) ? i : colorElem->GetIndexArray().GetAt(i);

				FbxColor c = colorElem->GetDirectArray().GetAt(index);
				pMeshData->meshVertices[index].color = { (float)c.mRed, (float)c.mGreen, (float)c.mBlue , (float)c.mAlpha };
			}
		}
		else if (mapMode == FbxGeometryElement::eByPolygonVertex)
		{
			int polyCount = pMesh->GetPolygonCount();
			int polyVertexCounter = 0;

			for (int p = 0; p < polyCount; p++)
			{
				int vertexCount = pMesh->GetPolygonSize(p);

				for (int v = 0; v < vertexCount; v++)
				{
					int colorIndex = (refMode == FbxGeometryElement::eDirect) ? polyVertexCounter : colorElem->GetIndexArray().GetAt(polyVertexCounter);

					FbxColor c = colorElem->GetDirectArray().GetAt(colorIndex);
					pMeshData->meshVertices[colorIndex].color = { (float)c.mRed, (float)c.mGreen, (float)c.mBlue , (float)c.mAlpha };
					polyVertexCounter++;
				}
			}
		}
	}
}

void FBXLoader::ExtractMeshNormal(MeshData* pMeshData, fbxsdk::FbxMesh* pMesh)
{
	int32_t ctrlPointCount = pMesh->GetControlPointsCount();
	int32_t normalElementCount = pMesh->GetElementNormalCount();
	if (0 < normalElementCount)
	{
		FbxGeometryElementNormal* normalElement = pMesh->GetElementNormal(0);
		switch (normalElement->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (normalElement->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				for (int32_t n = 0; n < ctrlPointCount; ++n)
				{
					FbxVector4 tmp = normalElement->GetDirectArray().GetAt(n);
					pMeshData->meshVertices[n].normal = { (float)tmp[0], (float)tmp[1], (float)tmp[2], (float)tmp[3] };
				}
			} break;
			case FbxGeometryElement::eIndexToDirect:
			{
				for (int32_t n = 0; n < ctrlPointCount; ++n)
				{
					int index = normalElement->GetIndexArray().GetAt(n);
					FbxVector4 tmp = normalElement->GetDirectArray().GetAt(index);
					pMeshData->meshVertices[n].normal = { (float)tmp[0], (float)tmp[1], (float)tmp[2], (float)tmp[3] };
				}
			} break;
			default:
				DEBUG_BREAK();
			} break;
		case FbxGeometryElement::eByPolygonVertex:
			switch (normalElement->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				//DEBUG_BREAK();
			} break;
			case FbxGeometryElement::eIndexToDirect:
			{
				DEBUG_BREAK();
			} break;
			default:
				DEBUG_BREAK();
			} break;
		}
	}
}

void FBXLoader::ExtractMeshTangent(MeshData* pMeshData, fbxsdk::FbxMesh* pMesh)
{
	// 탄젠트 정보
	int32_t ctrlPointCount = pMesh->GetControlPointsCount();
	int32_t tangentElementCount = pMesh->GetElementTangentCount();
	if (0 < tangentElementCount)
	{
		FbxGeometryElementTangent* tangenElement = pMesh->GetElementTangent(0);
		switch (tangenElement->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (tangenElement->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				for (int32_t n = 0; n < ctrlPointCount; ++n)
				{
					FbxVector4 tmp = tangenElement->GetDirectArray().GetAt(n);
					pMeshData->meshVertices[n].tangent = { (float)tmp[0], (float)tmp[1], (float)tmp[2], (float)tmp[3] };

					//DirectX::XMVECTOR nVec = DirectX::XMLoadFloat3(&(*outVertices)[n].normal);
					//DirectX::XMFLOAT3 floatTangent = DirectX::XMFLOAT3((*outVertices)[n].tangent.x, (*outVertices)[n].tangent.y, (*outVertices)[n].tangent.z);
					//DirectX::XMVECTOR tVec = DirectX::XMLoadFloat3(&floatTangent);
					//DirectX::XMVECTOR bVec = DirectX::XMVector3Cross(nVec, tVec);
					//DirectX::XMVECTOR c = DirectX::XMVector3Cross(tVec, bVec);
					//float dotVal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(c, nVec));
					//float handedness = (dotVal < 0.0f) ? -1.0f : 1.0f;
					//(*outVertices)[n].tangent.z = handedness;
				}
			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				for (int32_t n = 0; n < ctrlPointCount; ++n)
				{
					int32_t tangentIndex = tangenElement->GetIndexArray().GetAt(n);
					FbxVector4 tmp = tangenElement->GetDirectArray().GetAt(tangentIndex);
					pMeshData->meshVertices[n].tangent = { (float)tmp[0], (float)tmp[1], (float)tmp[2], (float)tmp[3] };

					//DirectX::XMVECTOR nVec = DirectX::XMLoadFloat3(&(*outVertices)[n].normal);
					//DirectX::XMFLOAT3 floatTangent = DirectX::XMFLOAT3((*outVertices)[n].tangent.x, (*outVertices)[n].tangent.y, (*outVertices)[n].tangent.z);
					//DirectX::XMVECTOR tVec = DirectX::XMLoadFloat3(&floatTangent);
					//DirectX::XMVECTOR bVec = DirectX::XMVector3Cross(nVec, tVec);
					//DirectX::XMVECTOR c = DirectX::XMVector3Cross(tVec, bVec);
					//float dotVal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(c, nVec));
					//float handedness = (dotVal < 0.0f) ? -1.0f : 1.0f;
					//(*outVertices)[n].tangent.z = handedness;
				}
			}
			break;
			default:
				DEBUG_BREAK();
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			switch (tangenElement->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect: DEBUG_BREAK(); break;
			case FbxGeometryElement::eIndexToDirect: DEBUG_BREAK(); break;
			default:
				DEBUG_BREAK();
			}
			break;
		}
	}
	else
	{
		// 탄젠트가 없을 경우.
		//DEBUG_BREAK();
		for (int32_t n = 0; n < ctrlPointCount; ++n)
		{
			DirectX::XMFLOAT4 tmpNormal4 = pMeshData->meshVertices[n].normal;
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

			pMeshData->meshVertices[n].tangent = { t3.x, t3.y, t3.z, 1.f };

			DirectX::XMVECTOR bVec = DirectX::XMVector3Cross(normalV, tVec);
			DirectX::XMVECTOR c = DirectX::XMVector3Cross(tVec, bVec);
			float dotVal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(c, normalV));
			float handedness = (dotVal < 0.0f) ? -1.0f : 1.0f;
			pMeshData->meshVertices[n].tangent.z = handedness;
		}
	}
}

void FBXLoader::ExtractMeshUV(MeshData* pMeshData, fbxsdk::FbxMesh* pMesh)
{
	int32_t ctrlPointCount = pMesh->GetControlPointsCount();
	int32_t uvElementCount = pMesh->GetElementUVCount();
	if (0 < uvElementCount)
	{
		FbxGeometryElementUV* uvElement = pMesh->GetElementUV(0);
		switch (uvElement->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
			switch (uvElement->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				for (int32_t n = 0; n < ctrlPointCount; ++n)
				{
					FbxVector2 uv = uvElement->GetDirectArray().GetAt(n);
					pMeshData->meshVertices[n].uv = { (float)uv[0], (float)uv[1] };
				}
			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				for (int32_t n = 0; n < ctrlPointCount; ++n)
				{
					int32_t uvIndex = uvIndex = uvElement->GetIndexArray().GetAt(n);
					FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
					pMeshData->meshVertices[n].uv = { (float)uv[0], (float)uv[1] };
				}
			}
			break;
			default:
				DEBUG_BREAK();
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect)
			{
				DEBUG_BREAK();

				/*int polygonCount = pMesh->GetPolygonCount();
				int polygonVertexCounter = 0;

				for (int poly = 0; poly < polygonCount; ++poly)
				{
					int polySize = pMesh->GetPolygonSize(poly);

					for (int vert = 0; vert < polySize; ++vert)
					{
						FbxVector2 uv = uvElement->GetDirectArray().GetAt(polygonVertexCounter);
						pMeshData->meshVertices[polygonVertexCounter].uv =
						{
							(float)uv[0],
							(float)uv[1]
						};

						polygonVertexCounter++;
					}
				}*/
			}
			else if (uvElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
			{
				//int polygonCount = pMesh->GetPolygonCount();
				//int polygonVertexCounter = 0;

				//for (int poly = 0; poly < polygonCount; ++poly)
				//{
				//	int polySize = pMesh->GetPolygonSize(poly);

				//	for (int vert = 0; vert < polySize; ++vert)
				//	{
				//		int uvIndex = uvElement->GetIndexArray().GetAt(polygonVertexCounter);
				//		FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);

				//		pMeshData->meshVertices[polygonVertexCounter].uv =
				//		{
				//			(float)uv[0],
				//			(float)uv[1]
				//		};

				//		polygonVertexCounter++;
				//	}
				//}
			}
			else
			{
				DEBUG_BREAK();
			}

			/*switch (uvElement->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				DEBUG_BREAK();
			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				DEBUG_BREAK();
			}
			break;
			default:
				DEBUG_BREAK();
			}
			break;*/
		}
	}
}


bool FBXLoader::Test(FBXMesh* pOutMesh, const std::string& file)
{
	if (nullptr == pOutMesh)
	{
		DEBUG_BREAK();
		return false;
	}

	if (!Init(file))
	{
		DEBUG_BREAK();
		return false;
	}

	pOutMesh->pData_->meshVertices.clear();
	pOutMesh->pData_->meshIndices.clear();
	if (pOutMesh->pData_->pVertexBuffer)
	{
		pOutMesh->pData_->pVertexBuffer->Release();
		pOutMesh->pData_->pVertexBuffer = nullptr;
	}
	if (pOutMesh->pData_->pIndexBuffer)
	{
		pOutMesh->pData_->pIndexBuffer->Release();
		pOutMesh->pData_->pIndexBuffer = nullptr;
	}
	pOutMesh->boneMap_.clear();
	pOutMesh->bones_.clear();
	pOutMesh->skinData_.clear();


	fbxsdk::FbxNode* pRootNode = pScene_->GetRootNode();
	int totalMeshCount = CountMeshes(pRootNode);
	if (1 != totalMeshCount)
	{
		DEBUG_BREAK();
	}

	fbxsdk::FbxMesh* pMesh = FindMesh(pRootNode);
	if (nullptr == pMesh)
	{
		DEBUG_BREAK();
		return false;
	}

	int polygonCount = pMesh->GetPolygonCount();
	FbxVector4* controlPoints = pMesh->GetControlPoints();

	pOutMesh->pData_->meshVertices.reserve(polygonCount * 3);
	pOutMesh->pData_->meshIndices.reserve(polygonCount * 3);

	std::unordered_map<SimpleVertex, uint32_t> vertexCache;
	std::vector<int> vertexCpIndexCache;
	vertexCpIndexCache.reserve(polygonCount * 3);

	
	int polygonVertexCounter = 0;
	for (int poly = 0; poly < polygonCount; ++poly)
	{
		int polySize = pMesh->GetPolygonSize(poly);
		if (polySize != 3)
		{
			DEBUG_BREAK();
		}

		for (int vert = 0; vert < polySize; ++vert)
		{
			int cpIndex = pMesh->GetPolygonVertex(poly, vert);

			// 여기서 모든 속성 추출
			SimpleVertex v;
			v.position.x = controlPoints[cpIndex][0];
			v.position.y = controlPoints[cpIndex][1];
			v.position.z = controlPoints[cpIndex][2];

			FbxVector4 normal;
			bool res1 = GetNormal(&normal, pMesh, cpIndex, polygonVertexCounter);
			v.normal.x = normal[0];
			v.normal.y = normal[1];
			v.normal.z = normal[2];
			v.normal.w = normal[3];

			FbxVector4 tangent;
			bool res2 = GetTangent(&tangent, pMesh, cpIndex, polygonVertexCounter);
			v.tangent.x = tangent[0];
			v.tangent.y = tangent[1];
			v.tangent.z = tangent[2];
			v.tangent.w = tangent[3];

			FbxVector2 uv;
			bool res3 = GetUV(&uv, pMesh, cpIndex, polygonVertexCounter);
			v.uv.x = uv[0];
			v.uv.y = uv[1];
			
			FbxColor color;
			bool res4 = GetColor(&color, pMesh, cpIndex, polygonVertexCounter);
			v.color.x = color.mRed;
			v.color.y = color.mGreen;
			v.color.z = color.mBlue;
			v.color.w = color.mAlpha;
			
			polygonVertexCounter++;

			uint32_t vertexIndex;
			auto iter = vertexCache.find(v);
			if (iter != vertexCache.end())
			{
				// cache에 이미 존재.
				vertexIndex = iter->second;
				pOutMesh->pData_->meshIndices.push_back(vertexIndex);
			}
			else
			{
				// cache에 존재 하지 않음.
				vertexIndex = pOutMesh->pData_->meshVertices.size();
				pOutMesh->pData_->meshVertices.push_back(v);
				pOutMesh->pData_->meshIndices.push_back(vertexIndex);

				vertexCpIndexCache.push_back(cpIndex);
				vertexCache[v] = vertexIndex;
			}
		}
	}

	FindBones(pRootNode, -1, pOutMesh);

	FindSkinWeight(pOutMesh, pMesh, pOutMesh->skinData_, pOutMesh->boneMap_);

	SkinDataToVertexData(pOutMesh, vertexCpIndexCache);


	int cpCount = pMesh->GetControlPointsCount();
	int block = 9999;

	return true;
}


bool FBXLoader::GetNormal(
	FbxVector4* outNormal,
	FbxMesh* mesh,
	int cpIndex,
	int polygonVertexIndex)
{
	FbxGeometryElementNormal* element = mesh->GetElementNormal();

	if (!element) return false;
		
	int index = 0;

	switch (element->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		index = cpIndex;
		break;

	case FbxGeometryElement::eByPolygonVertex:
		index = polygonVertexIndex;
		break;

	default:
		return false;
	}

	switch (element->GetReferenceMode())
	{
	case FbxGeometryElement::eDirect:
	{
		*outNormal = element->GetDirectArray().GetAt(index);
		return true;
	}
	case FbxGeometryElement::eIndexToDirect:
	{
		int directIndex = element->GetIndexArray().GetAt(index);
		*outNormal = element->GetDirectArray().GetAt(directIndex);
		return true;
	}

	default:
		return false;
	}
}

bool FBXLoader::GetTangent(FbxVector4* outTangent, FbxMesh* mesh, int cpIndex, int polygonVertexIndex)
{
	FbxGeometryElementTangent* element = mesh->GetElementTangent();

	if (!element) return false;

	int index = 0;

	switch (element->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		index = cpIndex;
		break;

	case FbxGeometryElement::eByPolygonVertex:
		index = polygonVertexIndex;
		break;

	default:
		return false;
	}

	switch (element->GetReferenceMode())
	{
	case FbxGeometryElement::eDirect:
	{
		*outTangent = element->GetDirectArray().GetAt(index);
		return true;
	}
	case FbxGeometryElement::eIndexToDirect:
	{
		int directIndex = element->GetIndexArray().GetAt(index);
		*outTangent = element->GetDirectArray().GetAt(directIndex);
		return true;
	}

	default:
		return false;
	}
}

bool FBXLoader::GetUV(
	FbxVector2* outUV,
	FbxMesh* mesh,
	int cpIndex,
	int polygonVertexIndex)
{
	FbxGeometryElementUV* element = mesh->GetElementUV();

	if (!element) return false;

	int index = 0;
	switch (element->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		index = cpIndex;
		break;

	case FbxGeometryElement::eByPolygonVertex:
		index = polygonVertexIndex;
		break;

	default:
		return false;
	}

	switch (element->GetReferenceMode())
	{
	case FbxGeometryElement::eDirect:
		*outUV = element->GetDirectArray().GetAt(index);
		return true;

	case FbxGeometryElement::eIndexToDirect:
	{
		int directIndex = element->GetIndexArray().GetAt(index);
		*outUV = element->GetDirectArray().GetAt(directIndex);
		return true;
	}

	default:
		return false;
	}
}

bool FBXLoader::GetColor(FbxColor* outColor, FbxMesh* mesh, int cpIndex, int polygonVertexIndex)
{
	if (mesh->GetElementVertexColorCount() == 0)
	{
		return false;
	}

	FbxGeometryElementVertexColor* element = mesh->GetElementVertexColor(0);
	int index = 0;

	switch (element->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
		index = cpIndex;
		break;

	case FbxGeometryElement::eByPolygonVertex:
		index = polygonVertexIndex;
		break;

	default:
		return false;
	}

	switch (element->GetReferenceMode())
	{
	case FbxGeometryElement::eDirect:
		if (index >= element->GetDirectArray().GetCount())
		{
			DEBUG_BREAK();
			return false;
		}
		*outColor = element->GetDirectArray().GetAt(index);
		return true;

	case FbxGeometryElement::eIndex:
	case FbxGeometryElement::eIndexToDirect:
	{
		if (index >= element->GetIndexArray().GetCount())
		{
			DEBUG_BREAK();
			return false;
		}

		int directIndex = element->GetIndexArray().GetAt(index);

		if (directIndex >= element->GetDirectArray().GetCount())
		{
			DEBUG_BREAK();
			return false;
		}

		*outColor = element->GetDirectArray().GetAt(directIndex);
		return true;
	}

	default:
		return false;
	}
}


void FBXLoader::FindBones(FbxNode* node, int parentBoneIndex, FBXMesh* pOutMesh)
{
	if (!node) return;

	FbxNodeAttribute* attr = node->GetNodeAttribute();
	int currentBoneIndex = parentBoneIndex;

	if (attr && attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Bone bone;
		bone.name = node->GetName();
		bone.parentIndex = parentBoneIndex;

		// 바인드 포즈 (로컬)
		//bone.localBindPose = node->EvaluateLocalTransform();
		// 바인드 포즈 (글로벌)
		//bone.globalBindPose = node->EvaluateGlobalTransform();

		currentBoneIndex = static_cast<int>(pOutMesh->bones_.size());
		pOutMesh->bones_.push_back(bone);
		pOutMesh->boneMap_[node] = currentBoneIndex;
	}

	const int childCount = node->GetChildCount();
	for (int i = 0; i < childCount; ++i)
	{
		FindBones(node->GetChild(i), currentBoneIndex, pOutMesh);
	}
}


void FBXLoader::FindSkinWeight(FBXMesh* pOutMesh, FbxMesh* mesh, std::vector<VertexSkinData>& outSkinData, const std::unordered_map<FbxNode*, int>& boneIndexMap)
{
	const int controlPointCount = mesh->GetControlPointsCount();
	outSkinData.resize(controlPointCount);

	const int deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
	if (deformerCount == 0)
	{
		DEBUG_BREAK();
		return;
	}

	for (int d = 0; d < deformerCount; d++)
	{
		FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(d, FbxDeformer::eSkin));
		const int clusterCount = skin->GetClusterCount();

		for (int c = 0; c < clusterCount; c++)
		{
			FbxCluster* cluster = skin->GetCluster(c);
			FbxNode* boneNode = cluster->GetLink();

			cluster->GetTransformMatrix(pOutMesh->bones_[c].meshBindPose);
			cluster->GetTransformLinkMatrix(pOutMesh->bones_[c].boneBindPose);

			if (!boneNode) continue;

			// Bone index (엔진에서 관리)
			auto it = boneIndexMap.find(boneNode);
			if (it == boneIndexMap.end()) continue;

			// 현재 cluster에 매칭되는 BoneIndex
			int boneIndex = it->second;

			// 현재 cluster에 영향받고있는 Indices들.
			const int indexCount = cluster->GetControlPointIndicesCount();	// 현재 cluster에 영향받고있는 index의 갯수
			const int* indices = cluster->GetControlPointIndices();
			const double* weights = cluster->GetControlPointWeights();	// index에 해당하는 weight

			for (int i = 0; i < indexCount; i++)
			{
				int cpIndex = indices[i];
				float weight = static_cast<float>(weights[i]);

				if (weight <= 0.0f) continue;
				AddBoneWeight(outSkinData[cpIndex], boneIndex, weight);
			}
		}
	}

	NormalizeSkinWeights(outSkinData);
}

void FBXLoader::AddBoneWeight(VertexSkinData& skinData, int boneIndex, float weight)
{
	for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		if (skinData.boneWeights[i] == 0.0f)
		{
			skinData.boneIndices[i] = boneIndex;
			skinData.boneWeights[i] = weight;
			return;
		}
	}

	//// 이미 4개가 찼다면 가장 작은 weight 교체
	int minIndex = 0;
	for (int i = 1; i < MAX_BONE_INFLUENCE; i++)
	{
		if (skinData.boneWeights[i] < skinData.boneWeights[minIndex])
		{
			minIndex = i;
		}
	}

	if (skinData.boneWeights[minIndex] < weight)
	{
		skinData.boneIndices[minIndex] = boneIndex;
		skinData.boneWeights[minIndex] = weight;
	}
}

void FBXLoader::NormalizeSkinWeights(std::vector<VertexSkinData>& skinData)
{
	for (auto& skinData : skinData)
	{
		float sum = 0.0f;
		for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
		{
			sum += skinData.boneWeights[i];
		}

		if (sum > 0.0f)
		{
			for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
			{
				skinData.boneWeights[i] /= sum;
			}
		}
	}
}

void FBXLoader::SkinDataToVertexData(FBXMesh* pOutMesh, const std::vector<int>& vertexCpIndexCache)
{
	for (int i = 0; i < pOutMesh->pData_->meshVertices.size(); ++i)
	{
		int cpIndex = vertexCpIndexCache[i];
		memcpy(pOutMesh->pData_->meshVertices[i].boneIndices, pOutMesh->skinData_[cpIndex].boneIndices, sizeof(UINT) * 4);
		memcpy(pOutMesh->pData_->meshVertices[i].blendWeights, pOutMesh->skinData_[cpIndex].boneWeights, sizeof(float) * 4);
	}
}

void FBXLoader::SkinDataToVertexData(FBXMesh* pOutMesh)
{
	pOutMesh->pData_->meshVertices;
	pOutMesh->skinData_;

	int a = pOutMesh->pData_->meshVertices.size();
	int b = pOutMesh->skinData_.size();


	if (a != b)
	{
		DEBUG_BREAK();
		return;
	}

	for (int i = 0; i < pOutMesh->skinData_.size(); ++i)
	{
		pOutMesh->pData_->meshVertices[i].blendWeights[0] = pOutMesh->skinData_[i].boneWeights[0];
		pOutMesh->pData_->meshVertices[i].blendWeights[1] = pOutMesh->skinData_[i].boneWeights[1];
		pOutMesh->pData_->meshVertices[i].blendWeights[2] = pOutMesh->skinData_[i].boneWeights[2];
		pOutMesh->pData_->meshVertices[i].blendWeights[3] = pOutMesh->skinData_[i].boneWeights[3];

		pOutMesh->pData_->meshVertices[i].boneIndices[0] = pOutMesh->skinData_[i].boneIndices[0];
		pOutMesh->pData_->meshVertices[i].boneIndices[1] = pOutMesh->skinData_[i].boneIndices[1];
		pOutMesh->pData_->meshVertices[i].boneIndices[2] = pOutMesh->skinData_[i].boneIndices[2];
		pOutMesh->pData_->meshVertices[i].boneIndices[3] = pOutMesh->skinData_[i].boneIndices[3];
	}
}


int FBXLoader::CountMeshes(fbxsdk::FbxNode* node)
{
	int count = 0;

	// 현재 노드가 Mesh인지 검사
	if (node->GetMesh()) count++;

	// 자식 노드 순회
	for (int i = 0; i < node->GetChildCount(); i++)
	{
		count += CountMeshes(node->GetChild(i));
	}

	return count;
}

bool FBXLoader::LoadAnimation(FBXAnimation* outAnimation, const std::string& file, double samplingRate)
{
	if (!Init(file))
	{
		DEBUG_BREAK();
		return false;
	}

	std::vector<FbxNode*> skeletonNodes;
	CollectSkeletonNodes(pScene_->GetRootNode(), skeletonNodes);

	// 1. AnimStack 가져오기
	int animStackCount = pScene_->GetSrcObjectCount<FbxAnimStack>();
	if (animStackCount == 0)
	{
		DEBUG_BREAK();
		return false;
	}

	FbxAnimStack* animStack = pScene_->GetSrcObject<FbxAnimStack>(0);
	pScene_->SetCurrentAnimationStack(animStack);

	outAnimation->animationClip_.name = animStack->GetName();

	// 2. 시간 정보
	FbxTakeInfo* takeInfo = pScene_->GetTakeInfo(animStack->GetName());
	FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
	FbxTime end = takeInfo->mLocalTimeSpan.GetStop();

	outAnimation->animationClip_.duration = end.GetSecondDouble() - start.GetSecondDouble();

	FbxTime::EMode timeMode = pScene_->GetGlobalSettings().GetTimeMode();
	outAnimation->animationClip_.frameRate = FbxTime::GetFrameRate(timeMode);

	// 3. Bone Animation 초기화
	const int boneCount = static_cast<int>(skeletonNodes.size());
	outAnimation->animationClip_.boneAnimations.resize(boneCount);
	for (int i = 0; i < boneCount; ++i)
	{
		outAnimation->animationClip_.boneAnimations[i].boneName = skeletonNodes[i]->GetName();
	}

	// 4. 시간 샘플링
	for (FbxTime t = start; t <= end; t += FbxTimeSeconds(samplingRate))
	{
		for (int b = 0; b < boneCount; b++)
		{
			FbxNode* boneNode = skeletonNodes[b];

			BoneKeyframe key;
			key.time = t;
			key.localTransform = boneNode->EvaluateLocalTransform(t);
			key.globalTransform = boneNode->EvaluateGlobalTransform(t);
			outAnimation->animationClip_.boneAnimations[b].keyframes.push_back(key);
		}
	}

	return true;
}

void FBXLoader::CollectSkeletonNodes(FbxNode* node, std::vector<FbxNode*>& outBones)
{
	if (!node) return;

	FbxNodeAttribute* attr = node->GetNodeAttribute();
	if (attr && attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		outBones.push_back(node);
	}

	for (int i = 0; i < node->GetChildCount(); ++i)
	{
		CollectSkeletonNodes(node->GetChild(i), outBones);
	}
}

SceneAxisInfo FBXLoader::GetSceneAxisInfo(fbxsdk::FbxScene* pScene)
{
	SceneAxisInfo axisInfo{};

	if (!pScene) return axisInfo;

	fbxsdk::FbxGlobalSettings& globalSettings = pScene->GetGlobalSettings();
	fbxsdk::FbxAxisSystem axisSystem = globalSettings.GetAxisSystem();

	// Up Vector
	axisInfo.UpAxis = axisSystem.GetUpVector(axisInfo.UpSign);

	// Front Vector
	axisInfo.FrontAxis = axisSystem.GetFrontVector(axisInfo.FrontSign);

	// Coordinate System (Left / Right Handed)
	axisInfo.CoordSystem = axisSystem.GetCoorSystem();

	// Unit (cm 기준)
	FbxSystemUnit systemUnit = globalSettings.GetSystemUnit();
	axisInfo.UnitScale = systemUnit.GetScaleFactor();

	return axisInfo;
}

void FBXLoader::CleanUp()
{
	if (pScene_)
	{
		pScene_->Destroy();
		pScene_ = nullptr;
	}
	if (pImporter_)
	{
		pImporter_->Destroy();
		pImporter_ = nullptr;
	}
	if (pIOSetting_)
	{
		pIOSetting_->Destroy();
		pIOSetting_ = nullptr;
	}
	if (pManager_)
	{
		pManager_->Destroy();
		pManager_ = nullptr;
	}
}
