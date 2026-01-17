#pragma once


class FBXMesh;
class FBXAnimation;
struct VertexSkinData;

class FBXLoader
{
public:
	FBXLoader();
	~FBXLoader();

	bool Init(const std::string& file);

	// Mesh
	bool LoadMesh(FBXMesh* pOutMesh, const std::string& file);

	FbxMesh* FindMesh(FbxNode* node);

	void FindBones(FbxNode* node, int parentBoneIndex, FBXMesh* pOutMesh);

	void FindSkinWeight
	(
		FbxMesh* mesh,
		std::vector<VertexSkinData>& outSkinData,
		const std::unordered_map<FbxNode*, int>& boneIndexMap
	);
	void AddBoneWeight(VertexSkinData& skinData, int boneIndex, float weight);
	void NormalizeSkinWeights(std::vector<VertexSkinData>& skinData);
	void SkinDataToVertexData(FBXMesh* pOutMesh);
	
	int CountMeshes(FbxNode* node);

	// Animation
	bool LoadAnimation
	(
		FBXAnimation* outAnimation, 
		const std::string& file, 
		//const std::vector<FbxNode*>& skeletonNodes, 
		double samplingRate = 1.0 / 60.0
	);
	void CollectSkeletonNodes(FbxNode* node, std::vector<FbxNode*>& outBones);

	void CleanUp();

	void Test(FbxNode* node);

	FbxManager* pManager_;
	FbxIOSettings* pIOSetting_;
	FbxImporter* pImporter_;
	FbxScene* pScene_;
};


//void ProcessMesh(FbxNode* inNode) 
//{
//	FbxMesh* currMesh = inNode->GetMesh(); 
//	mTriangleCount = currMesh->GetPolygonCount(); 
//	int vertexCounter = 0; 
//	mTriangles.reserve(mTriangleCount); 
//	for (unsigned int i = 0; i < mTriangleCount; ++i) 
//	{
//		XMFLOAT3 normal[3]; 
//		XMFLOAT3 tangent[3]; 
//		XMFLOAT3 binormal[3]; 
//		XMFLOAT2 UV[3][2]; 
//		Triangle currTriangle; 
//		mTriangles.push_back(currTriangle); 
//		for (unsigned int j = 0; j < 3; ++j) 
//		{
//			int ctrlPointIndex = currMesh->GetPolygonVertex(i, j); CtrlPoint* currCtrlPoint = mControlPoints[ctrlPointIndex]; ReadNormal(currMesh, ctrlPointIndex, vertexCounter, normal[j]); 
//			// We only have diffuse texture 
//			for (int k = 0; k < 1; ++k) 
//			{
//				ReadUV(currMesh, ctrlPointIndex, currMesh->GetTextureUVIndex(i, j), k, UV[j][k]); 
//			} 
//			PNTIWVertex temp; 
//			temp.mPosition = currCtrlPoint->mPosition; 
//			temp.mNormal = normal[j]; temp.mUV = UV[j][0]; // Copy the blending info from each control point 
//			for(unsigned int i = 0; i < currCtrlPoint->mBlendingInfo.size(); ++i)
//			{ 
//				VertexBlendingInfo currBlendingInfo; 
//				currBlendingInfo.mBlendingIndex = currCtrlPoint->mBlendingInfo[i].mBlendingIndex;
//				currBlendingInfo.mBlendingWeight = currCtrlPoint->mBlendingInfo[i].mBlendingWeight; 
//				temp.mVertexBlendingInfos.push_back(currBlendingInfo); 
//			} // Sort the blending info so that later we can remove // duplicated vertices 
//			temp.SortBlendingInfoByWeight(); 
//			mVertices.push_back(temp); 
//			mTriangles.back().mIndices.push_back(vertexCounter); 
//			++vertexCounter; 
//		} 
//	} // Now mControlPoints has served its purpose // We can free its memory 
//	for(auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr) 
//	{ 
//		delete itr->second;
//	} 
//
//	mControlPoints.clear(); 
//}



// FbxScene
// ¦¦¦¡ FbxNode(RootNode)
//     ¦§¦¡ Transform
//     ¦¢  ¦§¦¡ LclTranslation
//     ¦¢  ¦§¦¡ LclRotation
//     ¦¢  ¦¦¦¡ LclScaling
//     ¦¢
//     ¦§¦¡ FbxNode(Child Node)
//     ¦¢  ¦§¦¡ Transform
//     ¦¢  ¦¢
//     ¦¢  ¦§¦¡ FbxNodeAttribute -> [FbxGeometry] -> [FbxMesh]
//     ¦¢  ¦¢  ¦¢
//     ¦¢  ¦¢  ¦§¦¡ ControlPoints(Vertices)
//     ¦¢  ¦¢  ¦¢
//     ¦¢  ¦¢  ¦§¦¡ PolygonVertexIndices
//     ¦¢  ¦¢  ¦¢
//     ¦¢  ¦¢  ¦§¦¡ FbxLayer[0]
//     ¦¢  ¦¢  ¦¢  ¦§¦¡ FbxLayerElementNormal
//     ¦¢  ¦¢  ¦¢  ¦§¦¡ FbxLayerElementUV
//     ¦¢  ¦¢  ¦¢  ¦§¦¡ FbxLayerElementTangent
//     ¦¢  ¦¢  ¦¢  ¦§¦¡ FbxLayerElementBinormal
//     ¦¢  ¦¢  ¦¢  ¦§¦¡ FbxLayerElementColor
//     ¦¢  ¦¢  ¦¢  ¦¦¦¡ FbxLayerElementMaterial
//     ¦¢  ¦¢  ¦¢
//     ¦¢  ¦¢  ¦§¦¡ FbxLayer[1]
//     ¦¢  ¦¢  ¦¢  ¦¦¦¡(Additional UV / Color µî)
//     ¦¢  ¦¢  ¦¢
//     ¦¢  ¦¢  ¦§¦¡ FbxDeformer[0] -> [FbxSkin]
//     ¦¢  ¦¢  ¦¢   ¦§¦¡ FbxCluster[0]
//     ¦¢  ¦¢  ¦¢   ¦¢  ¦§¦¡ Linked FbxNode(Bone)
//     ¦¢  ¦¢  ¦¢   ¦¢  ¦¢  ¦¦¦¡ FbxNodeAttribute
//     ¦¢  ¦¢  ¦¢   ¦¢  ¦¢     ¦¦¦¡ FbxSkeleton
//     ¦¢  ¦¢  ¦¢   ¦¢  ¦¢
//     ¦¢  ¦¢  ¦¢   ¦¢  ¦§¦¡ ControlPointIndices[]
//     ¦¢  ¦¢  ¦¢   ¦¢  ¦§¦¡ ControlPointWeights[]
//     ¦¢  ¦¢  ¦¢   ¦¢  ¦§¦¡ TransformMatrix
//     ¦¢  ¦¢  ¦¢   ¦¢  ¦¦¦¡ TransformLinkMatrix
//     ¦¢  ¦¢  ¦¢   ¦¢
//     ¦¢  ¦¢  ¦¢   ¦¦¦¡ FbxCluster[N]
//     ¦¢  ¦¢  ¦¢
//     ¦¢  ¦¢  ¦¦¦¡ FbxDeformer[N]
//     ¦¢  ¦¢
//     ¦¢  ¦¦¦¡(Other Attribute Types)
//     ¦¢     ¦§¦¡ FbxSkeleton
//     ¦¢     ¦§¦¡ FbxCamera
//     ¦¢     ¦¦¦¡ FbxLight
//     ¦¢
//     ¦¦¦¡ FbxNode(Skeleton Node)
//        ¦§¦¡ Transform
//        ¦¢
//        ¦¦¦¡ FbxNodeAttribute -> [FbxSkeleton]
//			 ¦§¦¡ SkeletonType(Root / Limb / LimbNode)
//			 ¦¦¦¡ Size



//	FbxScene
//	¦¢
//	¦§¦¡ GlobalSettings
//	¦¢    ¦§¦¡ AxisSystem
//	¦¢    ¦§¦¡ SystemUnit
//	¦¢    ¦¦¦¡ TimeMode(24 / 30 / 60 fps)
//	¦¢
//	¦§¦¡ Scene Graph(Spatial Structure)
//	¦¢    ¦¢
//	¦¢    ¦§¦¡ FbxNode(Root)
//	¦¢    ¦¢    ¦§¦¡ FbxNodeAttribute(eNull)
//	¦¢    ¦¢    ¦§¦¡ LclTranslation
//	¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurveNode
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurve(X)
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurve(Y)
//	¦¢    ¦¢    ¦¢    ¦¢    ¦¦¦¡ FbxAnimCurve(Z)
//	¦¢    ¦¢    ¦¢    ¦¦¦¡(No Animation ¡æ Static Value)
//	¦¢    ¦¢    ¦¢
//	¦¢    ¦¢    ¦§¦¡ LclRotation
//	¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurveNode
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurve(X)
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurve(Y)
//	¦¢    ¦¢    ¦¢    ¦¢    ¦¦¦¡ FbxAnimCurve(Z)
//	¦¢    ¦¢    ¦¢    ¦¦¦¡ PreRotation / PostRotation / RotationOrder
//	¦¢    ¦¢    ¦¢
//	¦¢    ¦¢    ¦§¦¡ LclScaling
//	¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurveNode
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurve(X)
//	¦¢    ¦¢    ¦¢    ¦¢    ¦¢    ¦¦¦¡ Keyframes(time, value, tangent)
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurve(Y)
//	¦¢    ¦¢    ¦¢    ¦¢    ¦¦¦¡ FbxAnimCurve(Z)
//	¦¢    ¦¢    ¦¢    ¦¦¦¡ ScalingPivot / ScalingOffset
//	¦¢    ¦¢    ¦¢
//	¦¢    ¦¢    ¦§¦¡ Visibility
//	¦¢    ¦¢    ¦¢    ¦¦¦¡ FbxAnimCurve(optional)
//	¦¢    ¦¢    ¦¢
//	¦¢    ¦¢    ¦§¦¡ GeometricTranslation
//	¦¢    ¦¢    ¦§¦¡ GeometricRotation
//	¦¢    ¦¢    ¦¦¦¡ GeometricScaling
//	¦¢    ¦¢
//	¦¢    ¦§¦¡ FbxNode(Skeleton Root)
//	¦¢    ¦¢    ¦§¦¡ Attribute : FbxSkeleton(eRoot)
//	¦¢    ¦¢    ¦§¦¡ Parent : Scene Root
//	¦¢    ¦¢    ¦§¦¡ Children : Bone Nodes
//	¦¢    ¦¢    ¦¦¦¡(Same Transform + Curve Structure)
//	¦¢    ¦¢
//	¦¢    ¦§¦¡ FbxNode(Bone)
//	¦¢    ¦¢    ¦§¦¡ Attribute : FbxSkeleton(eLimbNode)
//	¦¢    ¦¢    ¦§¦¡ Parent : Skeleton Root
//	¦¢    ¦¢    ¦§¦¡ Children : Child Bones
//	¦¢    ¦¢    ¦¦¦¡(Same Transform + Curve Structure)
//	¦¢    ¦¢
//	¦¢    ¦¦¦¡ FbxNode(Mesh)
//	¦¢         ¦§¦¡ Attribute : FbxMesh
//	¦¢         ¦§¦¡ Parent : Root or Bone
//	¦¢         ¦§¦¡ GeometricTransform
//	¦¢         ¦¦¦¡ FbxSkin
//	¦¢              ¦¦¦¡ FbxCluster(Bone)
//	¦¢
//	¦§¦¡ Animation Data(Temporal Structure)
//	¦¢    ¦¢
//	¦¢    ¦§¦¡ FbxAnimStack(Walk)
//	¦¢    ¦¢    ¦§¦¡ FbxAnimLayer(Base Layer)
//	¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurveNode(LclTranslation)
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ Curve X
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ Curve Y
//	¦¢    ¦¢    ¦¢    ¦¢    ¦¦¦¡ Curve Z
//	¦¢    ¦¢    ¦¢    ¦¢
//	¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurveNode(LclRotation)
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ Curve X
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ Curve Y
//	¦¢    ¦¢    ¦¢    ¦¢    ¦¦¦¡ Curve Z
//	¦¢    ¦¢    ¦¢    ¦¢
//	¦¢    ¦¢    ¦¢    ¦§¦¡ FbxAnimCurveNode(LclScaling)
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ Curve X
//	¦¢    ¦¢    ¦¢    ¦¢    ¦§¦¡ Curve Y
//	¦¢    ¦¢    ¦¢    ¦¢    ¦¦¦¡ Curve Z
//	¦¢    ¦¢    ¦¢    ¦¢
//	¦¢    ¦¢    ¦¢    ¦¦¦¡(Optional Extra Curves)
//	¦¢    ¦¢    ¦¢
//	¦¢    ¦¢    ¦§¦¡ FbxAnimLayer(Additive Layer)
//	¦¢    ¦¢    ¦¢    ¦§¦¡ Same CurveNode Structure
//	¦¢    ¦¢    ¦¢    ¦¦¦¡ BlendMode / Weight
//	¦¢    ¦¢    ¦¢
//	¦¢    ¦¢    ¦¦¦¡ FbxAnimLayer(Override Layer)
//	¦¢    ¦¢
//	¦¢    ¦§¦¡ FbxAnimStack(Run)
//	¦¢    ¦¢    ¦¦¦¡(Same Layer Structure)
//	¦¢    ¦¢
//	¦¢    ¦¦¦¡ FbxTakeInfo
//	¦¢         ¦§¦¡ Take "Walk"
//	¦¢         ¦¢    ¦¦¦¡ TimeSpan(Start / End)
//	¦¢         ¦¢
//	¦¢         ¦¦¦¡ Take "Run"
//	¦¢              ¦¦¦¡ TimeSpan
//	¦¢
//	¦¦¦¡(Other Scene Objects)
//	¦§¦¡ Camera Nodes
//	¦§¦¡ Light Nodes
//	¦¦¦¡ Constraint Nodes(IK, Aim, Parent)