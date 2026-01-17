#include <iostream>
#include <fstream>
#include <vector>
#include <fbxsdk.h>

#pragma comment(lib, "alembic-md")
#pragma comment(lib, "libfbxsdk-md")
#pragma comment(lib, "libxml2-md")
#pragma comment(lib, "zlib-md")


//void DumpMesh(FbxMesh* mesh, std::ofstream& out, int depth)
//{
//    std::string indent(depth * 2, ' ');
//    out << indent << "[Mesh]\n";
//    out << indent << " Vertices : " << mesh->GetControlPointsCount() << "\n";
//    out << indent << " Polygons : " << mesh->GetPolygonCount() << "\n";
//}
//
//void DumpSkeleton(FbxSkeleton* skel, std::ofstream& out, int depth)
//{
//    std::string indent(depth * 2, ' ');
//    out << indent << "[Skeleton]\n";
//    out << indent << " Type : " << skel->GetSkeletonType() << "\n";
//}
//void DumpSkin(FbxMesh* mesh, std::ofstream& out)
//{
//    int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
//    out << " Skin Count: " << skinCount << "\n";
//}
//
//void DumpMaterials(FbxScene* scene, std::ofstream& out)
//{
//    int count = scene->GetMaterialCount();
//    out << "\n=== Materials ===\n";
//
//    for (int i = 0; i < count; ++i)
//    {
//        FbxSurfaceMaterial* mat = scene->GetMaterial(i);
//        out << mat->GetName() << "\n";
//    }
//}
//void DumpAnimations(FbxScene* scene, std::ofstream& out)
//{
//    int stackCount = scene->GetSrcObjectCount<FbxAnimStack>();
//    out << "\n=== Animations ===\n";
//
//    for (int i = 0; i < stackCount; ++i)
//    {
//        FbxAnimStack* stack = scene->GetSrcObject<FbxAnimStack>(i);
//        out << "[AnimStack] " << stack->GetName() << "\n";
//    }
//}
//void DumpAttribute(FbxNodeAttribute* attr, std::ofstream& out, int depth)
//{
//    if (!attr) return;
//
//    std::string indent(depth * 2, ' ');
//
//    out << indent << "[Attribute] Type: " << attr->GetAttributeType() << "\n";
//
//    switch (attr->GetAttributeType())
//    {
//    case FbxNodeAttribute::eMesh:
//        DumpMesh((FbxMesh*)attr, out, depth + 1);
//        break;
//    case FbxNodeAttribute::eSkeleton:
//        DumpSkeleton((FbxSkeleton*)attr, out, depth + 1);
//        break;
//    }
//}
//
//void DumpSceneSettings(FbxScene* scene, std::ofstream& out)
//{
//    out << "\n=== Scene Settings ===\n";
//
//    FbxGlobalSettings& gs = scene->GetGlobalSettings();
//
//    // Axis
//    FbxAxisSystem axis = gs.GetAxisSystem();
//    int sign;
//    auto up = axis.GetUpVector(sign);
//
//    out << "Up Axis : " << up << " sign=" << sign << "\n";
//    out << "Coord   : " << axis.GetCoorSystem() << "\n";
//
//    // Unit
//    out << "Unit    : " << gs.GetSystemUnit().GetScaleFactor() << " cm\n";
//
//    // Time
//    out << "TimeMode: " << gs.GetTimeMode() << "\n";
//}
//
//
//void DumpNodeRecursive(FbxNode* node, std::ofstream& out, int depth)
//{
//    if (!node) return;
//
//    std::string indent(depth * 2, ' ');
//    out << indent << "[Node] " << node->GetName() << "\n";
//
//    FbxAMatrix g = node->EvaluateGlobalTransform();
//    out << indent << " Global T: "
//        << g.GetT()[0] << ", "
//        << g.GetT()[1] << ", "
//        << g.GetT()[2] << "\n";
//
//    // Attribute
//    for (int i = 0; i < node->GetNodeAttributeCount(); ++i)
//    {
//        FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);
//        DumpAttribute(attr, out, depth + 1);
//    }
//
//    for (int i = 0; i < node->GetChildCount(); ++i)
//        DumpNodeRecursive(node->GetChild(i), out, depth + 1);
//
//}
//
//void DumpFBX(FbxScene* scene, const char* path)
//{
//    std::ofstream out(path);
//
//    DumpSceneSettings(scene, out);
//    DumpNodeRecursive(scene->GetRootNode(), out, 0);
//    DumpMaterials(scene, out);
//    DumpAnimations(scene, out);
//
//    out.close();
//}
//
//struct FBXContext
//{
//    FbxManager* manager = nullptr;
//    FbxScene* scene = nullptr;
//};
//
//bool InitFBX( const char* fbxPath, FBXContext& outCtx)
//{
//    // 1. Manager
//    outCtx.manager = FbxManager::Create();
//    if (!outCtx.manager)
//        return false;
//
//    // 2. IOSettings
//    FbxIOSettings* ios = FbxIOSettings::Create(outCtx.manager, IOSROOT);
//    outCtx.manager->SetIOSettings(ios);
//
//    // 3. Importer
//    FbxImporter* importer = FbxImporter::Create(outCtx.manager, "");
//
//    if (!importer->Initialize(fbxPath, -1, outCtx.manager->GetIOSettings()))
//    {
//        importer->Destroy();
//        return false;
//    }
//
//    // 4. Scene
//    outCtx.scene = FbxScene::Create(outCtx.manager, "Scene");
//    importer->Import(outCtx.scene);
//    importer->Destroy();
//
//    // 5. (권장) 좌표계 / 단위 변환
//    // DirectX 기준 (Left-Handed, Y-Up)
//    FbxAxisSystem::DirectX.ConvertScene(outCtx.scene);
//    FbxSystemUnit::cm.ConvertScene(outCtx.scene);
//
//    return true;
//}
//
//void ShutdownFBX(FBXContext& ctx)
//{
//    if (ctx.manager)
//    {
//        ctx.manager->Destroy();
//        ctx.manager = nullptr;
//        ctx.scene = nullptr;
//    }
//}


//struct FBXContext
//{
//    FbxManager* manager = nullptr;
//    FbxScene* scene = nullptr;
//};
//
///* =========================================================
//   Tree Indent Helper
//   ========================================================= */
//struct TreeIndent
//{
//    std::vector<bool> last;
//};
//
//void PrintTreePrefix(std::ofstream& out, const TreeIndent& indent)
//{
//    for (size_t i = 0; i < indent.last.size(); ++i)
//    {
//        if (i + 1 == indent.last.size())
//            out << (indent.last[i] ? "└─ " : "├─ ");
//        else
//            out << (indent.last[i] ? "   " : "│  ");
//    }
//}
//
///* =========================================================
//   FBX Init / Shutdown
//   ========================================================= */
//bool InitFBX(const char* path, FBXContext& ctx)
//{
//    ctx.manager = FbxManager::Create();
//    if (!ctx.manager)
//        return false;
//
//    FbxIOSettings* ios = FbxIOSettings::Create(ctx.manager, IOSROOT);
//    ctx.manager->SetIOSettings(ios);
//
//    FbxImporter* importer = FbxImporter::Create(ctx.manager, "");
//
//    if (!importer->Initialize(path, -1, ctx.manager->GetIOSettings()))
//    {
//        std::cout << importer->GetStatus().GetErrorString() << "\n";
//        importer->Destroy();
//        return false;
//    }
//
//    ctx.scene = FbxScene::Create(ctx.manager, "Scene");
//    importer->Import(ctx.scene);
//    importer->Destroy();
//
//    // 엔진 기준 변환 (DirectX)
//    FbxAxisSystem::DirectX.ConvertScene(ctx.scene);
//    FbxSystemUnit::cm.ConvertScene(ctx.scene);
//
//    return true;
//}
//
//void ShutdownFBX(FBXContext& ctx)
//{
//    if (ctx.manager)
//    {
//        ctx.manager->Destroy();
//        ctx.manager = nullptr;
//        ctx.scene = nullptr;
//    }
//}
//
///* =========================================================
//   Scene Settings Tree
//   ========================================================= */
//void DumpSceneSettingsTree(FbxScene* scene, std::ofstream& out, TreeIndent& indent)
//{
//    FbxAxisSystem axis = scene->GetGlobalSettings().GetAxisSystem();
//    int sign;
//
//    indent.last.push_back(false);
//    PrintTreePrefix(out, indent);
//    out << "AxisSystem\n";
//
//    indent.last.push_back(false);
//    PrintTreePrefix(out, indent);
//    out << "Up : " << axis.GetUpVector(sign) << "\n";
//    indent.last.pop_back();
//
//    indent.last.push_back(false);
//    PrintTreePrefix(out, indent);
//    out << "Front : " << axis.GetFrontVector(sign) << "\n";
//    indent.last.pop_back();
//
//    indent.last.push_back(true);
//    PrintTreePrefix(out, indent);
//    out << "Handed : "
//        << (axis.GetCoorSystem() == FbxAxisSystem::eRightHanded ? "Right" : "Left")
//        << "\n";
//    indent.last.pop_back();
//
//    indent.last.pop_back();
//
//    indent.last.push_back(true);
//    PrintTreePrefix(out, indent);
//    out << "Unit : "
//        << scene->GetGlobalSettings().GetSystemUnit().GetScaleFactor()
//        << " cm\n";
//    indent.last.pop_back();
//}
//
///* =========================================================
//   Node Attribute Tree
//   ========================================================= */
//const char* AttributeTypeToString(FbxNodeAttribute::EType type)
//{
//    switch (type)
//    {
//    case FbxNodeAttribute::eUnknown:        return "Unknown";
//    case FbxNodeAttribute::eNull:           return "Null";
//    case FbxNodeAttribute::eMarker:         return "Marker";
//    case FbxNodeAttribute::eSkeleton:       return "Skeleton";
//    case FbxNodeAttribute::eMesh:           return "Mesh";
//    case FbxNodeAttribute::eNurbs:          return "Nurbs";
//    case FbxNodeAttribute::ePatch:          return "Patch";
//    case FbxNodeAttribute::eCamera:         return "Camera";
//    case FbxNodeAttribute::eCameraStereo:   return "CameraStereo";
//    case FbxNodeAttribute::eCameraSwitcher: return "CameraSwitcher";
//    case FbxNodeAttribute::eLight:          return "Light";
//    case FbxNodeAttribute::eOpticalReference:return "OpticalReference";
//    case FbxNodeAttribute::eOpticalMarker:  return "OpticalMarker";
//    case FbxNodeAttribute::eNurbsCurve:     return "NurbsCurve";
//    case FbxNodeAttribute::eTrimNurbsSurface:return "TrimNurbsSurface";
//    case FbxNodeAttribute::eBoundary:       return "Boundary";
//    case FbxNodeAttribute::eNurbsSurface:   return "NurbsSurface";
//    case FbxNodeAttribute::eShape:          return "Shape (BlendShape)";
//    case FbxNodeAttribute::eLODGroup:       return "LODGroup";
//    case FbxNodeAttribute::eSubDiv:          return "SubDiv";
//    case FbxNodeAttribute::eCachedEffect:   return "CachedEffect";
//    case FbxNodeAttribute::eLine:            return "Line";
//    default:                                return "Unknown";
//    }
//}
//
//void DumpNodeAttributeTree(FbxNode* node, std::ofstream& out, TreeIndent& indent)
//{
//    int attrCount = node->GetNodeAttributeCount();
//    for (int i = 0; i < attrCount; ++i)
//    {
//        bool isLast = (i == attrCount - 1);
//        indent.last.push_back(isLast);
//
//        PrintTreePrefix(out, indent);
//
//        
//        FbxNodeAttribute* attr = node->GetNodeAttributeByIndex(i);
//        out << "[" << AttributeTypeToString(attr->GetAttributeType()) << "]\n";
//
//        indent.last.pop_back();
//    }
//}
//
///* =========================================================
//   Node Hierarchy Tree
//   ========================================================= */
//void DumpNodeTree(FbxNode* node, std::ofstream& out, TreeIndent& indent)
//{
//    if (!node) return;
//
//    int childCount = node->GetChildCount();
//    for (int i = 0; i < childCount; ++i)
//    {
//        FbxNode* child = node->GetChild(i);
//        bool isLast = (i == childCount - 1);
//
//        indent.last.push_back(isLast);
//        PrintTreePrefix(out, indent);
//        out << child->GetName() << "\n";
//
//        DumpNodeAttributeTree(child, out, indent);
//        DumpNodeTree(child, out, indent);
//
//        indent.last.pop_back();
//    }
//}
//
///* =========================================================
//   Animation Tree
//   ========================================================= */
//void DumpAnimationTree(FbxScene* scene, std::ofstream& out, TreeIndent& indent)
//{
//    int stackCount = scene->GetSrcObjectCount<FbxAnimStack>();
//
//    for (int i = 0; i < stackCount; ++i)
//    {
//        bool isLast = (i == stackCount - 1);
//        indent.last.push_back(isLast);
//
//        PrintTreePrefix(out, indent);
//        out << scene->GetSrcObject<FbxAnimStack>(i)->GetName() << "\n";
//
//        indent.last.pop_back();
//    }
//}
//
///* =========================================================
//   Full Dump Entry
//   ========================================================= */
//void DumpFBXTree(FbxScene* scene, const char* outPath)
//{
//    std::ofstream out(outPath);
//    TreeIndent indent;
//
//    out << "FBX SCENE\n";
//
//    // Global Settings
//    indent.last.push_back(false);
//    PrintTreePrefix(out, indent);
//    out << "GlobalSettings\n";
//    DumpSceneSettingsTree(scene, out, indent);
//    indent.last.pop_back();
//
//    // Nodes
//    indent.last.push_back(false);
//    PrintTreePrefix(out, indent);
//    out << "NodeHierarchy\n";
//    DumpNodeTree(scene->GetRootNode(), out, indent);
//    indent.last.pop_back();
//
//    // Animations
//    indent.last.push_back(true);
//    PrintTreePrefix(out, indent);
//    out << "Animations\n";
//    DumpAnimationTree(scene, out, indent);
//    indent.last.pop_back();
//
//    out.close();
//}
//
///* =========================================================
//   MAIN (사용 코드)
//   ========================================================= */
//int main()
//{
//    FBXContext ctx;
//
//    if (!InitFBX("..\\..\\Resource\\fbx\\Mesh\\JUMPER_MESH.FBX", ctx))
//    {
//        std::cout << "FBX Load Failed\n";
//        return 0;
//    }
//
//    DumpFBXTree(ctx.scene, "fbx_tree_dump.txt");
//
//    ShutdownFBX(ctx);
//    return 0;
//}



const char* GetAttributeTypeName(FbxNodeAttribute::EType type)
{
    switch (type)
    {
    case FbxNodeAttribute::eMesh:       return "Mesh";
    case FbxNodeAttribute::eSkeleton:   return "Skeleton";
    case FbxNodeAttribute::eCamera:     return "Camera";
    case FbxNodeAttribute::eLight:      return "Light";
    case FbxNodeAttribute::eNull:       return "Null";
    case FbxNodeAttribute::eMarker:     return "Marker";
    case FbxNodeAttribute::eNurbs:      return "Nurbs";
    case FbxNodeAttribute::ePatch:      return "Patch";
    default:                            return "Unknown";
    }
}

void PrintTreePrefix(std::ofstream& out,
    const std::vector<bool>& ancestorHasNext,
    bool isLast)
{
    for (bool hasNext : ancestorHasNext)
    {
        out << (hasNext ? "│  " : "   ");
    }
    out << (isLast ? "└─ " : "├─ ");
}

void DumpProperties(FbxObject* object, std::ofstream& out, int indent)
{
    for (FbxProperty prop = object->GetFirstProperty();
        prop.IsValid();
        prop = object->GetNextProperty(prop))
    {
        for (int i = 0; i < indent; ++i) out << "   ";

        out << "Property: " << prop.GetName();

        if (prop.GetPropertyDataType().GetType() == eFbxDouble3)
        {
            FbxDouble3 v = prop.Get<FbxDouble3>();
            out << " = (" << v[0] << ", " << v[1] << ", " << v[2] << ")";
        }
        else if (prop.GetPropertyDataType().GetType() == eFbxDouble)
        {
            out << " = " << prop.Get<FbxDouble>();
        }
        else if (prop.GetPropertyDataType().GetType() == eFbxString)
        {
            out << " = " << prop.Get<FbxString>().Buffer();
        }

        out << "\n";
    }
}

void DumpMeshDetails(FbxMesh* mesh, std::ofstream& out, int indent)
{
    int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
    for (int i = 0; i < skinCount; ++i)
    {
        auto* skin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));

        for (int j = 0; j < indent; ++j) out << "   ";
        out << "Deformer: Skin\n";

        int clusterCount = skin->GetClusterCount();
        for (int c = 0; c < clusterCount; ++c)
        {
            auto* cluster = skin->GetCluster(c);
            for (int j = 0; j < indent + 1; ++j) out << "   ";
            out << "Cluster: " << cluster->GetLink()->GetName() << "\n";
        }
    }
}

//void DumpNode(FbxNode* node, std::ofstream& out, int depth)
//{
//    for (int i = 0; i < depth; ++i) out << "│  ";
//    out << "└─ Node: " << node->GetName() << "\n";
//
//    // Node Attributes
//    int attrCount = node->GetNodeAttributeCount();
//    for (int i = 0; i < attrCount; ++i)
//    {
//        auto* attr = node->GetNodeAttributeByIndex(i);
//        for (int j = 0; j < depth + 1; ++j) out << "│  ";
//        out << "Attribute: " << GetAttributeTypeName(attr->GetAttributeType()) << "\n";
//
//        if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
//        {
//            DumpMeshDetails((FbxMesh*)attr, out, depth + 2);
//        }
//    }
//
//    DumpProperties(node, out, depth + 1);
//
//    // Children
//    int childCount = node->GetChildCount();
//    for (int i = 0; i < childCount; ++i)
//    {
//        DumpNode(node->GetChild(i), out, depth + 1);
//    }
//}

void DumpNode(
    FbxNode* node,
    std::ofstream& out,
    std::vector<bool> ancestorHasNext,
    bool isLast
)
{
    PrintTreePrefix(out, ancestorHasNext, isLast);
    out << "Node: " << node->GetName() << "\n";

    // === NodeAttribute ===
    int attrCount = node->GetNodeAttributeCount();
    for (int i = 0; i < attrCount; ++i)
    {
        bool attrLast = (i == attrCount - 1);

        PrintTreePrefix(out, ancestorHasNext, false);
        out << (attrLast ? "└─ " : "├─ ");
        out << "Attribute: "
            << GetAttributeTypeName(
                node->GetNodeAttributeByIndex(i)->GetAttributeType())
            << "\n";
    }

    // === Children ===
    int childCount = node->GetChildCount();
    for (int i = 0; i < childCount; ++i)
    {
        bool childIsLast = (i == childCount - 1);

        auto nextAncestors = ancestorHasNext;
        nextAncestors.push_back(!isLast);

        DumpNode(
            node->GetChild(i),
            out,
            nextAncestors,
            childIsLast
        );
    }
}
void DumpFBXToText(const char* fbxPath, const char* outPath)
{
    FbxManager* manager = FbxManager::Create();
    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);

    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(fbxPath, -1, manager->GetIOSettings()))
        return;

    FbxScene* scene = FbxScene::Create(manager, "Scene");
    importer->Import(scene);
    importer->Destroy();

    std::ofstream out(outPath);
    out << "Scene\n";

    DumpNode(scene->GetRootNode(), out, {}, true);

    out.close();
    manager->Destroy();
}

int main()
{
    DumpFBXToText("..\\..\\Resource\\fbx\\Mesh\\JUMPER_MESH.FBX", "JUMPER_MESH.txt");
}