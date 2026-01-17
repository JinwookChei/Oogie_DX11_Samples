#pragma once

// Mesh
struct SimpleVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT4 normal;
	DirectX::XMFLOAT4 tangent;
	DirectX::XMFLOAT2 uv;

	UINT boneIndices[4];
	float blendWeights[4];
};

struct Bone
{
	std::string name;
	int parentIndex;                // 부모 본 인덱스 (-1이면 루트)

	//FbxAMatrix localBindPose;        // 로컬 바인드 포즈
	//FbxAMatrix globalBindPose;       // 글로벌 바인드 포즈

	FbxAMatrix   meshBindPose;       // Mesh Bind Global
	FbxAMatrix   boneBindPose;       // Bone Bind Global
	FbxAMatrix   offsetMatrix;       // Inverse Bind Pose
};


struct MeshData
{
	std::vector<SimpleVertex> meshVertices;
	std::vector<WORD> meshIndices;

	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;

	MeshData()
		: meshVertices()
		, meshIndices()
		, pVertexBuffer(nullptr)
		, pIndexBuffer(nullptr)
	{

	}
	~MeshData()
	{
		if (pVertexBuffer)
		{
			pVertexBuffer->Release();
			pVertexBuffer = nullptr;
		}

		if (pIndexBuffer)
		{
			pIndexBuffer->Release();
			pIndexBuffer = nullptr;
		}
	}
};

constexpr int MAX_BONE_INFLUENCE = 4;

struct VertexSkinData
{
	UINT   boneIndices[MAX_BONE_INFLUENCE] = { 0 };
	float boneWeights[MAX_BONE_INFLUENCE] = { 0.0f };
};

class FBXMesh
{
public:
	FBXMesh();
	~FBXMesh();

	void CleanUp();

	MeshData* pData_;

	std::vector<Bone> bones_;

	// Pair : <Node, boneIndex>
	std::unordered_map<FbxNode*, int> boneMap_;
	std::vector<VertexSkinData> skinData_;
};
