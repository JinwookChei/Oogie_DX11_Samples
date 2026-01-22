#pragma once

class FBXMesh;
class MeshData;
class FBXAnimation;
struct VertexSkinData;

struct SceneAxisInfo
{
	FbxAxisSystem::EUpVector UpAxis;
	int UpSign;        // +1 / -1
	FbxAxisSystem::EFrontVector FrontAxis;
	int FrontSign;    // +1 / -1
	FbxAxisSystem::ECoordSystem CoordSystem;
	double UnitScale; // cm 기준 스케일 (1.0 = cm)

public:

	void PrintSceneAxisInfo() 
	{
        // 1. Up 축 결정
        std::string upStr = (UpSign > 0 ? "+" : "-");
        int upIdx = 0; // 0:X, 1:Y, 2:Z
        if (UpAxis == FbxAxisSystem::eXAxis) { upStr += "X"; upIdx = 0; }
        else if (UpAxis == FbxAxisSystem::eYAxis) { upStr += "Y"; upIdx = 1; }
        else { upStr += "Z"; upIdx = 2; }

        // 2. Front 축 계산 (UpAxis와 Parity 조합)
        // ParityEven(1) 이면 Up의 다음 축, ParityOdd(2) 이면 Up의 다음다음 축
        int frontIdx = 0;
        if (FrontAxis == FbxAxisSystem::eParityEven) 
        {
            frontIdx = (upIdx + 1) % 3;
        }
        else 
        {
            frontIdx = (upIdx + 2) % 3;
        }

        std::string frontStr = (FrontSign > 0 ? "+" : "-");
        if (frontIdx == 0) frontStr += "X";
        else if (frontIdx == 1) frontStr += "Y";
        else frontStr += "Z";

        auto AxisVector = [](int idx, int sign) {
            FbxVector4 v(0, 0, 0);
            v[idx] = (double)sign;
            return v;
            };

        FbxVector4 upVec = AxisVector(upIdx, UpSign);
        FbxVector4 frontVec = AxisVector(frontIdx, FrontSign);
        FbxVector4 rightVec;

        if (CoordSystem == FbxAxisSystem::eRightHanded) {
            rightVec = frontVec.CrossProduct(upVec);
        }
        else {
            rightVec = upVec.CrossProduct(frontVec);
        }

        int rightIdx = 0;
        int rightSign = 1;

        if (fabs(rightVec[0]) > 0.5) { rightIdx = 0; rightSign = (rightVec[0] > 0) ? 1 : -1; }
        else if (fabs(rightVec[1]) > 0.5) { rightIdx = 1; rightSign = (rightVec[1] > 0) ? 1 : -1; }
        else { rightIdx = 2; rightSign = (rightVec[2] > 0) ? 1 : -1; }

        std::string rightStr = (rightSign > 0 ? "+" : "-");
        if (rightIdx == 0) rightStr += "X";
        else if (rightIdx == 1) rightStr += "Y";
        else rightStr += "Z";

        // --- 직관적인 출력 ---
        std::stringstream ss;
        ss << "\n====================================\n";
        ss << " [FBX Scene World Orientation] \n";
        ss << "------------------------------------\n";
        ss << "  위쪽 (UP)    : " << upStr << "\n";
        ss << "  앞쪽 (FRONT) : " << frontStr << "\n";
        ss << "  오른쪽 (RIGHT) : " << rightStr << "\n";
        ss << "------------------------------------\n";
        ss << "  좌표계: " << (CoordSystem == FbxAxisSystem::eRightHanded ? "오른손(Right-Handed)" : "왼손(Left-Handed)") << "\n";
        ss << "  단위: " << UnitScale << " cm\n";
        ss << "====================================\n";

        OutputDebugStringA(ss.str().c_str());
	}
};

class FBXLoader
{
public:
	FBXLoader();
	~FBXLoader();

	bool Init(const std::string& file);

    // Mesh
    bool LoadMesh(FBXMesh* pOutMesh, const std::string& file);
    fbxsdk::FbxMesh* FindMesh(fbxsdk::FbxNode* node);
    void ExtractMeshData(MeshData* pMeshData, fbxsdk::FbxMesh* pMesh);
    void ExtractMeshColor(MeshData* pMeshData, fbxsdk::FbxMesh* pMesh);
    void ExtractMeshNormal(MeshData* pMeshData, fbxsdk::FbxMesh* pMesh);
    void ExtractMeshTangent(MeshData* pMeshData, fbxsdk::FbxMesh* pMesh);
    void ExtractMeshUV(MeshData* pMeshData, fbxsdk::FbxMesh* pMesh);

    
    bool Test(FBXMesh* pOutMesh, const std::string& file);
    bool GetNormal(FbxVector4* outNormal, FbxMesh* mesh, int cpIndex, int polygonVertexIndex);
    bool GetTangent(FbxVector4* outTangent, FbxMesh* mesh, int cpIndex, int polygonVertexIndex);
    bool GetUV(FbxVector2* outUV, FbxMesh* mesh, int cpIndex, int polygonVertexIndex);
    bool GetColor(FbxColor* outColor, FbxMesh* mesh, int cpIndex, int polygonVertexIndex);
    

    void FindBones(FbxNode* node, int parentBoneIndex, FBXMesh* pOutMesh);
    void FindSkinWeight
    (
        FBXMesh* pOutMesh,
        FbxMesh* mesh,
        std::vector<VertexSkinData>& outSkinData,
        const std::unordered_map<FbxNode*, int>& boneIndexMap
    );
    void AddBoneWeight(VertexSkinData& skinData, int boneIndex, float weight);
    void NormalizeSkinWeights(std::vector<VertexSkinData>& skinData);

    void SkinDataToVertexData(FBXMesh* pOutMesh, const std::vector<int>& vertexCpIndexCache);
    void SkinDataToVertexData(FBXMesh* pOutMesh);

    int CountMeshes(fbxsdk::FbxNode* node);


    // Animation
    bool LoadAnimation
    (
        FBXAnimation* outAnimation,
        const std::string& file,
        double samplingRate = 1.0 / 60.0
    );
    void CollectSkeletonNodes(FbxNode* node, std::vector<FbxNode*>& outBones);


	SceneAxisInfo GetSceneAxisInfo(fbxsdk::FbxScene* pScene);
	void CleanUp();



	fbxsdk::FbxManager* pManager_;
	fbxsdk::FbxIOSettings* pIOSetting_;
	fbxsdk::FbxImporter* pImporter_;
	fbxsdk::FbxScene* pScene_;

	SceneAxisInfo sceneAxisInfo_;
};



// FbxScene
// └─ FbxNode(RootNode)
//     ├─ Transform
//     │  ├─ LclTranslation
//     │  ├─ LclRotation
//     │  └─ LclScaling
//     │
//     ├─ FbxNode(Child Node)
//     │  ├─ Transform
//     │  │
//     │  ├─ FbxNodeAttribute -> [FbxGeometry] -> [FbxMesh]
//     │  │  │
//     │  │  ├─ ControlPoints(Vertices)
//     │  │  │
//     │  │  ├─ PolygonVertexIndices
//     │  │  │
//     │  │  ├─ FbxLayer[0]
//     │  │  │  ├─ FbxLayerElementNormal
//     │  │  │  ├─ FbxLayerElementUV
//     │  │  │  ├─ FbxLayerElementTangent
//     │  │  │  ├─ FbxLayerElementBinormal
//     │  │  │  ├─ FbxLayerElementColor
//     │  │  │  └─ FbxLayerElementMaterial
//     │  │  │
//     │  │  ├─ FbxLayer[1]
//     │  │  │  └─(Additional UV / Color 등)
//     │  │  │
//     │  │  ├─ FbxDeformer[0] -> [FbxSkin]
//     │  │  │   ├─ FbxCluster[0]
//     │  │  │   │  ├─ Linked FbxNode(Bone)
//     │  │  │   │  │  └─ FbxNodeAttribute
//     │  │  │   │  │     └─ FbxSkeleton
//     │  │  │   │  │
//     │  │  │   │  ├─ ControlPointIndices[]
//     │  │  │   │  ├─ ControlPointWeights[]
//     │  │  │   │  ├─ TransformMatrix
//     │  │  │   │  └─ TransformLinkMatrix
//     │  │  │   │
//     │  │  │   └─ FbxCluster[N]
//     │  │  │
//     │  │  └─ FbxDeformer[N]
//     │  │
//     │  └─(Other Attribute Types)
//     │     ├─ FbxSkeleton
//     │     ├─ FbxCamera
//     │     └─ FbxLight
//     │
//     └─ FbxNode(Skeleton Node)
//        ├─ Transform
//        │
//        └─ FbxNodeAttribute -> [FbxSkeleton]
//			 ├─ SkeletonType(Root / Limb / LimbNode)
//			 └─ Size



//	FbxScene
//	│
//	├─ GlobalSettings
//	│    ├─ AxisSystem
//	│    ├─ SystemUnit
//	│    └─ TimeMode(24 / 30 / 60 fps)
//	│
//	├─ Scene Graph(Spatial Structure)
//	│    │
//	│    ├─ FbxNode(Root)
//	│    │    ├─ FbxNodeAttribute(eNull)
//	│    │    ├─ LclTranslation
//	│    │    │    ├─ FbxAnimCurveNode
//	│    │    │    │    ├─ FbxAnimCurve(X)
//	│    │    │    │    ├─ FbxAnimCurve(Y)
//	│    │    │    │    └─ FbxAnimCurve(Z)
//	│    │    │    └─(No Animation → Static Value)
//	│    │    │
//	│    │    ├─ LclRotation
//	│    │    │    ├─ FbxAnimCurveNode
//	│    │    │    │    ├─ FbxAnimCurve(X)
//	│    │    │    │    ├─ FbxAnimCurve(Y)
//	│    │    │    │    └─ FbxAnimCurve(Z)
//	│    │    │    └─ PreRotation / PostRotation / RotationOrder
//	│    │    │
//	│    │    ├─ LclScaling
//	│    │    │    ├─ FbxAnimCurveNode
//	│    │    │    │    ├─ FbxAnimCurve(X)
//	│    │    │    │    │    └─ Keyframes(time, value, tangent)
//	│    │    │    │    ├─ FbxAnimCurve(Y)
//	│    │    │    │    └─ FbxAnimCurve(Z)
//	│    │    │    └─ ScalingPivot / ScalingOffset
//	│    │    │
//	│    │    ├─ Visibility
//	│    │    │    └─ FbxAnimCurve(optional)
//	│    │    │
//	│    │    ├─ GeometricTranslation
//	│    │    ├─ GeometricRotation
//	│    │    └─ GeometricScaling
//	│    │
//	│    ├─ FbxNode(Skeleton Root)
//	│    │    ├─ Attribute : FbxSkeleton(eRoot)
//	│    │    ├─ Parent : Scene Root
//	│    │    ├─ Children : Bone Nodes
//	│    │    └─(Same Transform + Curve Structure)
//	│    │
//	│    ├─ FbxNode(Bone)
//	│    │    ├─ Attribute : FbxSkeleton(eLimbNode)
//	│    │    ├─ Parent : Skeleton Root
//	│    │    ├─ Children : Child Bones
//	│    │    └─(Same Transform + Curve Structure)
//	│    │
//	│    └─ FbxNode(Mesh)
//	│         ├─ Attribute : FbxMesh
//	│         ├─ Parent : Root or Bone
//	│         ├─ GeometricTransform
//	│         └─ FbxSkin
//	│              └─ FbxCluster(Bone)
//	│
//	├─ Animation Data(Temporal Structure)
//	│    │
//	│    ├─ FbxAnimStack(Walk)
//	│    │    ├─ FbxAnimLayer(Base Layer)
//	│    │    │    ├─ FbxAnimCurveNode(LclTranslation)
//	│    │    │    │    ├─ Curve X
//	│    │    │    │    ├─ Curve Y
//	│    │    │    │    └─ Curve Z
//	│    │    │    │
//	│    │    │    ├─ FbxAnimCurveNode(LclRotation)
//	│    │    │    │    ├─ Curve X
//	│    │    │    │    ├─ Curve Y
//	│    │    │    │    └─ Curve Z
//	│    │    │    │
//	│    │    │    ├─ FbxAnimCurveNode(LclScaling)
//	│    │    │    │    ├─ Curve X
//	│    │    │    │    ├─ Curve Y
//	│    │    │    │    └─ Curve Z
//	│    │    │    │
//	│    │    │    └─(Optional Extra Curves)
//	│    │    │
//	│    │    ├─ FbxAnimLayer(Additive Layer)
//	│    │    │    ├─ Same CurveNode Structure
//	│    │    │    └─ BlendMode / Weight
//	│    │    │
//	│    │    └─ FbxAnimLayer(Override Layer)
//	│    │
//	│    ├─ FbxAnimStack(Run)
//	│    │    └─(Same Layer Structure)
//	│    │
//	│    └─ FbxTakeInfo
//	│         ├─ Take "Walk"
//	│         │    └─ TimeSpan(Start / End)
//	│         │
//	│         └─ Take "Run"
//	│              └─ TimeSpan
//	│
//	└─(Other Scene Objects)
//	├─ Camera Nodes
//	├─ Light Nodes
//	└─ Constraint Nodes(IK, Aim, Parent)