#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <DirectXTex.h>
#include <fbxsdk.h>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <sstream>
#include <string_view>
#include <functional>

#pragma comment(lib, "DirectXTex")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dxgi")


// FBX
#pragma comment(lib, "alembic-md")
#pragma comment(lib, "libfbxsdk-md")
#pragma comment(lib, "libxml2-md")
#pragma comment(lib, "zlib-md")


#ifdef _DEBUG
#define DEBUG_BREAK __debugbreak
#else
#define DEBUG_BREAK
#endif // DEBUG

#include "VectorType.h"
#include "Transform.h"
#include "FBXLoader.h"
#include "FBXAnimation.h"
#include "FBXMesh.h"


