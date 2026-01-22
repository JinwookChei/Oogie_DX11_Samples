#pragma once

constexpr int MAX_BONE_INFLUENCE = 4;

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

	// unordered_map 전용.
	bool operator==(const SimpleVertex& other) const
	{
		return position.x == other.position.x
			&& position.y == other.position.y
			&& position.z == other.position.z
			&& color.x == other.color.x
			&& color.y == other.color.y
			&& color.z == other.color.z
			&& color.w == other.color.w
			&& normal.x == other.normal.x
			&& normal.y == other.normal.y
			&& normal.z == other.normal.z
			&& normal.w == other.normal.w
			&& uv.x == other.uv.x
			&& uv.y == other.uv.y;
	}
};

//struct ControlPointKey
//{
//	fbxsdk::FbxVector4 cpPos;
//
//	ControlPointKey(fbxsdk::FbxVector4 _cpPos)
//	{
//		cpPos = _cpPos;
//	}
//
//	ControlPointKey& operator=(const fbxsdk::FbxVector4& other)
//	{
//		cpPos = other;
//		return *this;
//	}
//
//	bool operator==(const ControlPointKey& other) const
//	{
//		return cpPos[0] == other.cpPos[0]
//			&& cpPos[1] == other.cpPos[1]
//			&& cpPos[2] == other.cpPos[2]
//			&& cpPos[3] == other.cpPos[3];
//	}
//};

inline void HashCombine(size_t& seed, size_t value)
{
	seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

inline size_t HashFloat(float f)
{
	uint32_t bits;
	memcpy(&bits, &f, sizeof(float));
	return std::hash<uint32_t>{}(bits);
}

template<>
struct std::hash<SimpleVertex>
{
	size_t operator()(const SimpleVertex& v) const noexcept
	{
		size_t seed = 0;

		HashCombine(seed, HashFloat(v.position.x));
		HashCombine(seed, HashFloat(v.position.y));
		HashCombine(seed, HashFloat(v.position.z));

		HashCombine(seed, HashFloat(v.color.x));
		HashCombine(seed, HashFloat(v.color.y));
		HashCombine(seed, HashFloat(v.color.z));
		HashCombine(seed, HashFloat(v.color.w));

		HashCombine(seed, HashFloat(v.normal.x));
		HashCombine(seed, HashFloat(v.normal.y));
		HashCombine(seed, HashFloat(v.normal.z));
		HashCombine(seed, HashFloat(v.normal.w));

		HashCombine(seed, HashFloat(v.uv.x));
		HashCombine(seed, HashFloat(v.uv.y));

		return seed;
	}
};

//template<>
//struct std::hash<ControlPointKey>
//{
//	size_t operator()(const ControlPointKey& key) const noexcept
//	{
//		size_t seed = 0;
//		HashCombine(seed, HashFloat(key.cpPos[0]));
//		HashCombine(seed, HashFloat(key.cpPos[1]));
//		HashCombine(seed, HashFloat(key.cpPos[2]));
//		HashCombine(seed, HashFloat(key.cpPos[3]));
//		return seed;
//	}
//};

struct Bone
{
	std::string name;
	int parentIndex;                // 부모 본 인덱스 (-1이면 루트)

	FbxAMatrix   meshBindPose;       // Mesh Bind Global
	FbxAMatrix   boneBindPose;       // Bone Bind Global
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



struct VertexSkinData
{
	UINT   boneIndices[4] = { 0 };
	float boneWeights[4] = { 0.0f };
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
