#include "stdafx.h"
#include "FBXLoader.h"

FBXLoader::FBXLoader()
	:	pManager_(nullptr)
	,	pIOSetting_(nullptr)
	,	pImporter_(nullptr)
	,	pScene_(nullptr)
{
}

FBXLoader::~FBXLoader()
{
	CleanUp();
}


bool FBXLoader::Init(const std::string& file)
{
	CleanUp();

	pManager_ = FbxManager::Create();
	if (nullptr == pManager_)
	{
		DEBUG_BREAK();
		return false;
	}

	pIOSetting_ = FbxIOSettings::Create(pManager_, IOSROOT);
	pManager_->SetIOSettings(pIOSetting_);

	pImporter_ = FbxImporter::Create(pManager_, "fbxImporter");
	bool status = pImporter_->Initialize(file.c_str(), -1, pManager_->GetIOSettings());
	if (false == status)
	{
		DEBUG_BREAK();
		return false;
	}

	pScene_ = FbxScene::Create(pManager_, "fbxScene");
	pImporter_->Import(pScene_);

	FbxAxisSystem engineAxis(FbxAxisSystem::eZAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
	engineAxis.ConvertScene(pScene_);
	FbxGeometryConverter geomConv(pManager_);
	geomConv.Triangulate(pScene_, true);
}

bool FBXLoader::LoadMesh(FBXMesh* pOutMesh, const std::string& file)
{
	if (!Init(file))
	{
		DEBUG_BREAK();
		return false;
	}

	pOutMesh->pData_->meshVertices.clear();
	pOutMesh->pData_->meshIndices.clear();

	FbxNode* pRootNode = pScene_->GetRootNode();
	int totalMeshCount = CountMeshes(pRootNode);
	if (1 != totalMeshCount)
	{
		DEBUG_BREAK();
	}

	FbxMesh* pMesh = FindMesh(pRootNode);
	if (nullptr == pMesh)
	{
		DEBUG_BREAK();
		return false;
	}

	int32_t ctrlPointCount = pMesh->GetControlPointsCount();
	pOutMesh->pData_->meshVertices.reserve(ctrlPointCount);

	// 버텍스 수집
	for (int32_t n = 0; n < ctrlPointCount; ++n)
	{
		FbxVector4 p = pMesh->GetControlPointAt(n);
		SimpleVertex v;
		v.position = { (float)p[0], (float)p[1], (float)p[2] };
		pOutMesh->pData_->meshVertices.push_back(v);
	}

	// 인덱스 수집
	int32_t polygonCount = pMesh->GetPolygonCount();
	for (int32_t n = 0; n < polygonCount; ++n)
	{
		int32_t polygonSize = pMesh->GetPolygonSize(n);
		for (int32_t j = 0; j < polygonSize; ++j)
		{
			int32_t ctrlIndex = pMesh->GetPolygonVertex(n, j);
			pOutMesh->pData_->meshIndices.push_back(ctrlIndex);
		}
	}


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
				int index = (refMode == FbxGeometryElement::eDirect)
					? i : colorElem->GetIndexArray().GetAt(i);

				FbxColor c = colorElem->GetDirectArray().GetAt(index);
				pOutMesh->pData_->meshVertices[index].color = { (float)c.mRed, (float)c.mGreen, (float)c.mBlue , (float)c.mAlpha };
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
					int colorIndex = (refMode == FbxGeometryElement::eDirect)
						? polyVertexCounter : colorElem->GetIndexArray().GetAt(polyVertexCounter);

					FbxColor c = colorElem->GetDirectArray().GetAt(colorIndex);
					pOutMesh->pData_->meshVertices[colorIndex].color = { (float)c.mRed, (float)c.mGreen, (float)c.mBlue , (float)c.mAlpha };
					polyVertexCounter++;
				}
			}
		}
	}

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
					pOutMesh->pData_->meshVertices[n].normal = { (float)tmp[0], (float)tmp[1], (float)tmp[2], (float)tmp[3] };
				}
			} break;
			case FbxGeometryElement::eIndexToDirect:
			{
				for (int32_t n = 0; n < ctrlPointCount; ++n)
				{
					int index = normalElement->GetIndexArray().GetAt(n);
					FbxVector4 tmp = normalElement->GetDirectArray().GetAt(index);
					pOutMesh->pData_->meshVertices[n].normal = { (float)tmp[0], (float)tmp[1], (float)tmp[2], (float)tmp[3] };
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
				DEBUG_BREAK();
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

	// 탄젠트 정보
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
					pOutMesh->pData_->meshVertices[n].tangent = { (float)tmp[0], (float)tmp[1], (float)tmp[2], (float)tmp[3] };

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
					pOutMesh->pData_->meshVertices[n].tangent = { (float)tmp[0], (float)tmp[1], (float)tmp[2], (float)tmp[3] };

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
			break;
		}
	}
	else
	{
		// 탄젠트가 없을 경우.
		for (int32_t n = 0; n < ctrlPointCount; ++n)
		{
			DirectX::XMFLOAT4 tmpNormal4 = pOutMesh->pData_->meshVertices[n].normal;
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

			pOutMesh->pData_->meshVertices[n].tangent = { t3.x, t3.y, t3.z, 1.f };

			DirectX::XMVECTOR bVec = DirectX::XMVector3Cross(normalV, tVec);
			DirectX::XMVECTOR c = DirectX::XMVector3Cross(tVec, bVec);
			float dotVal = DirectX::XMVectorGetX(DirectX::XMVector3Dot(c, normalV));
			float handedness = (dotVal < 0.0f) ? -1.0f : 1.0f;
			pOutMesh->pData_->meshVertices[n].tangent.z = handedness;
		}
	}



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
					pOutMesh->pData_->meshVertices[n].uv = { (float)uv[0], (float)uv[1] };
				}
			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				for (int32_t n = 0; n < ctrlPointCount; ++n)
				{
					int32_t uvIndex = uvIndex = uvElement->GetIndexArray().GetAt(n);
					FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
					pOutMesh->pData_->meshVertices[n].uv = { (float)uv[0], (float)uv[1] };
				}
			}
			break;
			default:
				DEBUG_BREAK();
			}
			break;
		case FbxGeometryElement::eByPolygonVertex:
			switch (uvElement->GetReferenceMode())
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
			break;
		}
	}

	FindBones(pRootNode, -1, pOutMesh);

	FindSkinWeight(pMesh, pOutMesh->skinData_, pOutMesh->boneMap_);

	SkinDataToVertexData(pOutMesh);

	pOutMesh;

	return true;
}

FbxMesh* FBXLoader::FindMesh(FbxNode* node)
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
		FbxMesh* findMesh = FindMesh(node->GetChild(n));
		if (nullptr != findMesh) 
		{
			return findMesh;
		}
	}

	return nullptr;
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
		bone.localBindPose = node->EvaluateLocalTransform();

		// 바인드 포즈 (글로벌)
		bone.globalBindPose = node->EvaluateGlobalTransform();
		
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

void FBXLoader::FindSkinWeight(FbxMesh* mesh, std::vector<VertexSkinData>& outSkinData, const std::unordered_map<FbxNode*, int>& boneIndexMap)
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
			if (!boneNode)
				continue;

			// Bone index (엔진에서 관리)
			auto it = boneIndexMap.find(boneNode);
			if (it == boneIndexMap.end())
				continue;

			int boneIndex = it->second;

			const int* indices = cluster->GetControlPointIndices();
			const double* weights = cluster->GetControlPointWeights();
			const int indexCount = cluster->GetControlPointIndicesCount();

			for (int i = 0; i < indexCount; i++)
			{
				int cpIndex = indices[i];
				float weight = static_cast<float>(weights[i]);

				if (weight <= 0.0f)
					continue;

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

	// 이미 4개가 찼다면 가장 작은 weight 교체
	int minIndex = 0;
	for (int i = 1; i < MAX_BONE_INFLUENCE; i++)
	{
		if (skinData.boneWeights[i] < skinData.boneWeights[minIndex])
			minIndex = i;
	}

	if (skinData.boneWeights[minIndex] < weight)
	{
		skinData.boneIndices[minIndex] = boneIndex;
		skinData.boneWeights[minIndex] = weight;
	}
}

void FBXLoader::NormalizeSkinWeights(std::vector<VertexSkinData>& skinData)
{
	for (auto& v : skinData)
	{
		float sum = 0.0f;
		for (float w : v.boneWeights)
			sum += w;

		if (sum > 0.0f)
		{
			for (float& w : v.boneWeights)
				w /= sum;
		}
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


int FBXLoader::CountMeshes(FbxNode* node)
{
	int count = 0;

	// 현재 노드가 Mesh인지 검사
	if (node->GetMesh())
		count++;

	// 자식 노드 순회
	for (int i = 0; i < node->GetChildCount(); i++)
	{
		count += CountMeshes(node->GetChild(i));
	}

	return count;
}



bool FBXLoader::LoadAnimation
(
	FBXAnimation* outAnimation,
	const std::string& file,
	//const std::vector<FbxNode*>& skeletonNodes,
	double samplingRate /*= 1.0 / 60.0*/
)
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

	// 4. 시간 샘플링
	for (FbxTime t = start; t <= end; t += FbxTimeSeconds(samplingRate))
	{
		for (int b = 0; b < boneCount; b++)
		{
			FbxNode* boneNode = skeletonNodes[b];

			BoneKeyframe key;
			key.time = t;
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

void FBXLoader::CleanUp()
{
	if (nullptr != pScene_)
	{
		pScene_->Destroy();
		pScene_ = nullptr;
	};
	if (nullptr != pImporter_)
	{
		pImporter_->Destroy();
		pImporter_ = nullptr;
	};
	if (nullptr != pIOSetting_)
	{
		pIOSetting_->Destroy();
		pIOSetting_ = nullptr;
	};
	if (nullptr != pManager_)
	{
		pManager_->Destroy();
		pManager_ = nullptr;
	};
}

void FBXLoader::Test(FbxNode* node)
{
	FbxNodeAttribute* att = node->GetNodeAttribute();
	if (nullptr != att)
	{
		FbxNodeAttribute::EType type = att->GetAttributeType();

		if (type != FbxNodeAttribute::EType::eSkeleton)
		{
			int tmp2 = 20;
		}
		int tmp = 10;
	}

	for (int i = 0; i < node->GetChildCount(); ++i)
	{
		FbxNode* cNode = node->GetChild(i);
		Test(cNode);
	}
}
